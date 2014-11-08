// umoznuje vykreslit jeden objekt
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <pthread.h>
#include "core/ptr.hpp"
#include "render/mesh.hpp"
#include "render/framebuffer.hpp"
#include "taskgraph/singlethread_scheduler.hpp"
#include "scenegraph/scene.hpp"
#include "scenegraph/method.hpp"
#include "scenegraph/drawmesh.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/transforms_task.hpp"
#include "scenegraph/program_task.hpp"
#include "scenegraph/callmethod_task.hpp"
#include "scenegraph/foreach_task.hpp"
#include "ui/glut_window.hpp"
#include "resource/resource_manager.hpp"
#include "resource/orkmesh_loader.hpp"
#include "core/utils.hpp"
#include "resources.hpp"
#include "core/html_logger.hpp"

int const WIDTH = 800;
int const HEIGHT = 600;

struct P3_N3_UV_C
{
	float x, y, z, nx, ny, nz, u, v;
	float r, g, b, a;

	P3_N3_UV_C() {}
	P3_N3_UV_C(float x, float y, float z, float nx, float ny, float nz,
		float u, float v, float r, float g, float b, float a) :
		  x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), u(u), v(v), r(r), g(g), b(b), a(a)
	{}
};


class main_window	: public glut_window
{
public:
	typedef glut_window base;

	main_window();	
	void display(double t, double dt) override;
	void reshape(int w, int h) override;

private:
	ptr<resource> load_cube_mesh() const;
	void init_rerources(resource_manager & resman);

	GLuint _vao;
	scene_manager _scene;
	framebuffer & _fb;
};

main_window::main_window()
	: base(parameters().version(3, 3).size(WIDTH, HEIGHT).name("scene-graph test"))
	, _fb(framebuffer::default_fb())
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	logger::DEBUG_LOGGER = make_ptr<html_logger>("DEBUG", "log.html");
	logger::INFO_LOGGER = make_ptr<html_logger>("INFO", "log.html");
	logger::WARNING_LOGGER = make_ptr<html_logger>("WARNING", "log.html");
	logger::ERROR_LOGGER = make_ptr<html_logger>("ERROR", "log.html");

	ptr<resource_manager> resman = make_ptr<resource_manager>();
	init_rerources(*resman);

	_scene.resources(resman);
	_scene.scheduler(make_ptr<singlethread_scheduler>());

	ptr<scene_node> root = make_ptr<scene_node>("scene");

	ptr<scene_node> camera = make_ptr<scene_node>("camera");
	camera->append_flag("camera");
	camera->assoc_module("material", resman->load_resource<shader::module>("camera"));
	camera->assoc_method("draw", make_ptr<method>(resman->load_resource<task_factory>("camera_method")));
	root->append_child(camera);

	ptr<scene_node> cube = make_ptr<scene_node>("model");
	cube->append_flag("object");
	cube->assoc_mesh("geometry", resman->load_resource<mesh_buffers>("cube.mesh"));
	cube->assoc_module("material", resman->load_resource<shader::module>("plastic"));
	cube->assoc_method("draw", make_ptr<method>(resman->load_resource<task_factory>("object_method")));
	root->append_child(cube);

	_scene.root(root);
	_scene.camera_node(camera);
	_scene.camera_method("draw");
}

void main_window::display(double t, double dt)
{
	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	_scene.camera_node()->local_to_parent(glm::inverse(V));

	_fb.clear(true, true, false);

	_scene.update(0.0, 0.0);
	_scene.draw();

	base::display(t, dt);
}

void main_window::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	_fb.depth_test(true);

	_scene.camera_to_screen(glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f));

	base::reshape(w, h);
	// TODO: idle()
}

void main_window::init_rerources(resource_manager & resman)
{
	// camera
	ptr<module_resource> camera = make_ptr<module_resource>(330, read_file("shaders/camera.glsl").c_str());
	resman.insert_resource("camera", camera);  // module

//	<sequence>
//		<setTransforms module="this.material" worldPos="worldCameraPos"/>
//		<foreach var="o" flag="object" culling="true">
//			<callMethod name="$o.draw"/>
//		</foreach>
//	</sequence>
	std::vector<ptr<task_factory>> subtasks;
//	ptr<transforms_task_factory> camera_meth_transform = make_ptr<transforms_task_factory>(task_factory::qualified_name("this.material"), nullptr, "world_camera_pos");
//	subtasks.push_back(camera_meth_transform);
	ptr<call_method_task_factory> camera_meth_call = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	ptr<foreach_task_factory> camera_meth_fore = make_ptr<foreach_task_factory>("o", "object", camera_meth_call);
	subtasks.push_back(camera_meth_fore);
	ptr<sequence_task_factory_resource> camera_meth = make_ptr<sequence_task_factory_resource>(subtasks);
	resman.insert_resource("camera_method", camera_meth);

	// cube
	resman.insert_resource("cube.mesh", load_cube_mesh());

	ptr<module_resource> plastic = make_ptr<module_resource>(330, read_file("shaders/plastic.glsl").c_str());
	resman.insert_resource("plastic", plastic);  // module

//<sequence>
//	<setProgram>
//		<module name="camera.material"/>
//		<module name="this.material"/>
//	</setProgram>
//	<setTransforms localToScreen="localToScreen" localToWorld="localToWorld"/>
//	<drawMesh name="this.geometry"/>
//</sequence>
	subtasks.clear();
	std::vector<task_factory::qualified_name> module_names{task_factory::qualified_name("camera.material"), task_factory::qualified_name("this.material")};
	ptr<program_task_factory> object_meth_prog = make_ptr<program_task_factory>(module_names, false);
	subtasks.push_back(object_meth_prog);
	ptr<transforms_task_factory> object_meth_transform = make_ptr<transforms_task_factory>(task_factory::qualified_name(""), "local_to_screen", nullptr);
	subtasks.push_back(object_meth_transform);
	ptr<draw_mesh_task_factory> object_meth_draw_mesh = make_ptr<draw_mesh_task_factory>(task_factory::qualified_name("this.geometry"));
	subtasks.push_back(object_meth_draw_mesh);
	ptr<sequence_task_factory_resource> object_meth = make_ptr<sequence_task_factory_resource>(subtasks);
	resman.insert_resource("object_method", object_meth);  // method

	std::vector<ptr<shader::module>> modules{resman.load_resource<shader::module>("camera"), resman.load_resource<shader::module>("plastic")};
	ptr<program_resource> camera_plastic_prog = make_ptr<program_resource>(modules);
	resman.insert_resource("camera;plastic;", camera_plastic_prog);  // program
}

ptr<resource> main_window::load_cube_mesh() const
{
	ptr<mesh_buffers_resource> m = make_ptr<mesh_buffers_resource>();
	orkmesh_loader loader;
	loader.load("meshes/cube.mesh", m);  // TODO: naco mi je ten navratovy bool, zrus ho
	return m;
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	int i=pthread_getconcurrency();  // Inconsistency detected by ld.so: dl-version.c: 224: ...
	return 0;
}

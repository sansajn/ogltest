#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <pthread.h>
#include <AntTweakBar.h>
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
#include "resource/assimp_loader.hpp"

#include "tweakbar_resource.hpp"
#include "tweakbar_manager.hpp"
#include "draw_tweakbar_task.hpp"

using std::cout;

int const WIDTH = 800;
int const HEIGHT = 600;

char const MODEL_FILE[] = "meshes/monkey.ply";
char const SCRIPT_FILE[] = "scripts/tweakbar.lua";
char const PROGRAM_MODULES[] = "shaders/nolighting.vs;shader/nolighting.fs";

class app_window : public glut_window
{
public:
	typedef glut_window base;

	app_window();
	~app_window() {}

	void display(double t, double dt) override;
	void reshape(int w, int h) override;
	bool mouse_motion(int x, int y) override;
	bool mouse_passive_motion(int x, int y) override;
	bool mouse_wheel(wheel b, modifier m, int x, int y) override;
	bool mouse_click(button b, state s, modifier m, int x, int y) override;
	bool key_typed(unsigned char c, modifier m, int x, int y) override;
	bool key_released(unsigned char c, modifier m, int x, int y) override;
	bool special_key(key k, modifier m, int x, int y) override;
	bool special_key_released(key k, modifier m, int x, int y) override;

private:
	ptr<resource> load_cube_mesh() const;
	void init_resources(resource_manager & resman);
	void tweakbar_init();

	ptr<tweakbar_manager> _ui;

	scene_manager _scene;
	framebuffer & _fb;
	GLuint _vao;
};

app_window::app_window()
	: base(parameters().size(WIDTH, HEIGHT).name("AntTwekBar test"))
	, _fb(framebuffer::default_fb())
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

//	logger::DEBUG_LOGGER = make_ptr<html_logger>("DEBUG", "log.html");
//	logger::INFO_LOGGER = make_ptr<html_logger>("INFO", "log.html");
//	logger::WARNING_LOGGER = make_ptr<html_logger>("WARNING", "log.html");
//	logger::ERROR_LOGGER = make_ptr<html_logger>("ERROR", "log.html");

	ptr<resource_manager> resman = make_ptr<resource_manager>();
	init_resources(*resman);

	_scene.resources(resman);
	_scene.scheduler(make_ptr<singlethread_scheduler>());

	ptr<scene_node_resource> root = make_ptr<scene_node_resource>("scene");
	resman->insert_resource("scene", root);

	// camera
	ptr<scene_node> camera = make_ptr<scene_node>("camera");
	camera->append_flag("camera");
	camera->assoc_module("material", resman->load_resource<shader::module>("camera"));
	camera->assoc_method("draw", make_ptr<method>(resman->load_resource<task_factory>("camera_method")));
	root->append_child(camera);

	// model
	ptr<scene_node> model = make_ptr<scene_node>("model");
	model->append_flag("object");
	model->assoc_mesh("geometry", resman->load_resource<mesh_buffers>("cube.mesh"));
	model->assoc_module("material", resman->load_resource<shader::module>("plastic"));
	model->assoc_method("draw", make_ptr<method>(resman->load_resource<task_factory>("object_method")));
	root->append_child(model);

	// tweakbar
	ptr<scene_node> tweakbar = make_ptr<scene_node>("tweakbar");
	tweakbar->append_flag("overlay");
	tweakbar->assoc_method("draw", make_ptr<method>(resman->load_resource<task_factory>("tweakbar_method")));
	root->append_child(tweakbar);
	tweakbar_init();

	// TODO: pridaj root do resource-managera ako scene

	_scene.root(root);
	_scene.camera_node(camera);
	_scene.camera_method("draw");
}

void app_window::tweakbar_init()
{
	std::vector<twbar_variable_desc> vars = read_tweakbar_as_lua(SCRIPT_FILE);

	ptr<tweakbar_handler> handler = make_ptr<tweakbar_resource>(_scene.resources(), vars);

	tweakbar_manager::bar_data data(handler, false, true, 'o');
	std::vector<tweakbar_manager::bar_data> datas;
	datas.push_back(data);

	_ui = make_ptr<tweakbar_manager>(datas);
}

void app_window::display(double t, double dt)
{
	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	_scene.camera_node()->local_to_parent(glm::inverse(V));

	_fb.clear(true, true, false);

	glBindVertexArray(_vao);
	_scene.update(t, dt);
	_scene.draw();

	_ui->display(t, dt);
	base::display(t, dt);
}

void app_window::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	_fb.depth_test(true);

	_scene.camera_to_screen(glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f));

	_ui->reshape(w, h);
	base::reshape(w, h);
}

void app_window::init_resources(resource_manager & resman)
{
	// camera
	ptr<module_resource> camera = make_ptr<module_resource>(330, read_file("shaders/camera.glsl").c_str());
	resman.insert_resource("camera", camera);  // module

//	<sequence>
//		<foreach var="o" flag="object" culling="true">
//			<callMethod name="$o.draw"/>
//		</foreach>
//		<foreach var="o" flag="overlay">
//			<callMethod name="$o.draw"/>
//		</foreach>
//	</sequence>
	std::vector<ptr<task_factory>> subtasks;
	ptr<call_method_task_factory> camera_meth_call = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "object", camera_meth_call));
	ptr<call_method_task_factory> overlay_meth_call = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "overlay", overlay_meth_call));
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

	// tweakbar
	ptr<draw_tweakbar_factory_resource> tbar_meth = make_ptr<draw_tweakbar_factory_resource>();
	resman.insert_resource("tweakbar_method", tbar_meth);
}

ptr<resource> app_window::load_cube_mesh() const
{
	ptr<mesh_buffers_resource> m = make_ptr<mesh_buffers_resource>();
	orkmesh_loader loader;
	loader.load("meshes/cube.mesh", m);  // TODO: naco mi je ten navratovy bool, zrus ho
	return m;
}

bool app_window::mouse_motion(int x, int y)
{
	return _ui->mouse_motion(x, y);
}

bool app_window::mouse_passive_motion(int x, int y)
{
	return _ui->mouse_passive_motion(x, y);
}

bool app_window::mouse_wheel(wheel b, modifier m, int x, int y)
{
	return _ui->mouse_wheel(b, m, x, y);
}

bool app_window::mouse_click(button b, state s, modifier m, int x, int y)
{
	return _ui->mouse_click(b, s, m, x, y);
}

bool app_window::key_typed(unsigned char c, modifier m, int x, int y)
{
	return _ui->key_typed(c, m, x, y);
}

bool app_window::key_released(unsigned char c, modifier m, int x, int y)
{
	return _ui->key_released(c, m, x, y);
}

bool app_window::special_key(key k, modifier m, int x, int y)
{
	return _ui->special_key(k, m, x, y);
}

bool app_window::special_key_released(key k, modifier m, int x, int y)
{
	return _ui->special_key_released(k, m, x, y);
}

int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	int i=pthread_getconcurrency();  // Inconsistency detected by ld.so: dl-version.c: 224: ...
	return 0;
}

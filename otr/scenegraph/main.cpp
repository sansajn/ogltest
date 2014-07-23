// umoznuje vykreslit jeden objekt
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ptr.h"
#include "ui/sdl_window.h"
#include "render/mesh.h"
#include "render/framebuffer.h"
#include "scenegraph/scene.h"
#include "scenegraph/method.h"
#include "scenegraph/drawmesh.h"
#include "scenegraph/sequence_task.h"
#include "scenegraph/transforms_task.h"
#include "scenegraph/program_task.h"
#include "scenegraph/callmethod_task.h"
#include "scenegraph/foreach_task.h"

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


class dummy_camera_draw_factory : public task_factory
{
public:
	dummy_camera_draw_factory()
	{
		std::vector<std::string> modules;
		modules.push_back("shader/cubes_transf.vs");
		modules.push_back("shader/cubes.fs");
		_program_factory = make_ptr<program_task_factory>(modules);  // TODO: camera nepotrebuje program (bez neho failne transforms)
		_transforms_factory = make_ptr<transforms_task_factory>("local_to_screen");
		_method_task = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
		_foreach_factory = make_ptr<foreach_task_factory>("o", "object", _method_task);
	}

	ptr<task> create_task(ptr<scene_node> context)
	{
		std::vector<ptr<task_factory>> subtasks;
		subtasks.push_back(_program_factory);
		subtasks.push_back(_transforms_factory);
		subtasks.push_back(_foreach_factory);
		ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
		return seq->create_task(context);
	}

private:
	ptr<program_task_factory> _program_factory;
	ptr<transforms_task_factory> _transforms_factory;
	ptr<foreach_task_factory> _foreach_factory;
	ptr<call_method_task_factory> _method_task;
};


class main_window
	: public gl::sdl_window
{
public:
	main_window();	
	void display();
	void reshape(int w, int h);

private:
	ptr<method> create_object_method(ptr<scene_node> n) const;
	std::vector<std::string> object_modules() const;
	ptr<mesh_buffers> load_cube_mesh() const;

	GLuint _vao;
	scene_manager _scene;
};

main_window::main_window()
	: gl::sdl_window(gl::window::parameters().version(3, 3).size(WIDTH, HEIGHT).name("scene-graph test"))
{
	_scene.scheduler(make_ptr<singlethread_scheduler>());

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	ptr<scene_node> root = make_ptr<scene_node>();

	ptr<scene_node> cube = make_ptr<scene_node>();
	cube->append_flag("object");
	ptr<mesh_buffers> cube_mesh = load_cube_mesh();
	cube->assoc_mesh("geometry", cube_mesh);
	cube->assoc_method("draw", create_object_method(cube));
	root->append_child(cube);

	ptr<scene_node> camera = make_ptr<scene_node>();
	camera->append_flag("camera");
	ptr<dummy_camera_draw_factory> camera_draw = make_ptr<dummy_camera_draw_factory>();
	camera->assoc_method("draw", make_ptr<method>(camera_draw, camera));
	root->append_child(camera);

	_scene.camera_to_screen(
		glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f));

	_scene.root(root);
	_scene.camera_node(camera);
	_scene.camera_method("draw");

	frame_buffer & fb = frame_buffer::default_fb();
	fb.depth_test(true);
}

void main_window::display()
{
	frame_buffer & fb = frame_buffer::default_fb();
	fb.clear(true, true, false);

	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	_scene.camera_node()->local_to_parent(glm::inverse(V));

	_scene.update(0.0, 0.0);
	_scene.draw();

	gl::sdl_window::display();
}

void main_window::reshape(int w, int h)
{
	frame_buffer & fb = frame_buffer::default_fb();
	fb.depth_test(true);

	_scene.camera_to_screen(glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f));

	gl::sdl_window::reshape(w, h);
}

ptr<method> main_window::create_object_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	subtasks.push_back(make_ptr<program_task_factory>(object_modules()));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	subtasks.push_back(make_ptr<draw_mesh_factory>("geometry"));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

std::vector<std::string> main_window::object_modules() const
{
	return std::vector<std::string>{
		std::string("shader/cubes_transf.vs"), std::string("shader/cubes.fs")};
}

ptr<mesh_buffers> main_window::load_cube_mesh() const
{
	mesh<P3_N3_UV_C, unsigned int> cube(GL_TRIANGLES);
	cube.append_attribute_type(0, 3, GL_FLOAT, false);
	cube.append_attribute_type(1, 3, GL_FLOAT, false);
	cube.append_attribute_type(2, 2, GL_FLOAT, false);
	cube.append_attribute_type(3, 4, GL_FLOAT, false);
	cube.append_vertex(P3_N3_UV_C(-1, -1, +1, 0, 0, +1, 0, 0, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, +1, 0, 0, +1, 1, 0, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, +1, 0, 0, +1, 1, 1, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, +1, 0, 0, +1, 1, 1, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, +1, 0, 0, +1, 0, 1, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, +1, 0, 0, +1, 0, 0, 1, 0, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, +1, +1, 0, 0, 0, 0, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, -1, +1, 0, 0, 1, 0, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, -1, +1, 0, 0, 1, 1, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, -1, +1, 0, 0, 1, 1, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, +1, +1, 0, 0, 0, 1, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, +1, +1, 0, 0, 0, 0, 0, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, +1, 0, +1, 0, 0, 0, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, +1, 0, +1, 0, 1, 0, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, -1, 0, +1, 0, 1, 1, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, -1, 0, +1, 0, 1, 1, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, -1, 0, +1, 0, 0, 1, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, +1, 0, +1, 0, 0, 0, 0, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, -1, 0, 0, -1, 0, 0, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, -1, 0, 0, -1, 1, 0, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, -1, 0, 0, -1, 1, 1, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, -1, 0, 0, -1, 1, 1, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, +1, -1, 0, 0, -1, 0, 1, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, -1, 0, 0, -1, 0, 0, 0, 1, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, -1, -1, 0, 0, 0, 0, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, +1, -1, 0, 0, 1, 0, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, +1, -1, 0, 0, 1, 1, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, +1, -1, 0, 0, 1, 1, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, +1, -1, -1, 0, 0, 0, 1, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, -1, -1, 0, 0, 0, 0, 1, 0, 1, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, -1, 0, -1, 0, 0, 0, 1, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, -1, 0, -1, 0, 1, 0, 1, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, +1, 0, -1, 0, 1, 1, 1, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(+1, -1, +1, 0, -1, 0, 1, 1, 1, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, +1, 0, -1, 0, 0, 1, 1, 1, 0, 1));
	cube.append_vertex(P3_N3_UV_C(-1, -1, -1, 0, -1, 0, 0, 0, 1, 1, 0, 1));

	return cube.buf();
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

// zobrazi kocku za pouziti scene grafu
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ptr.hpp"
#include "ui/sdl_window.h"
#include "render/framebuffer.h"
#include "scenegraph/scene.hpp"
#include "scenegraph/method.hpp"
#include "scenegraph/drawmesh.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/transforms_task.hpp"
#include "scenegraph/program_task.hpp"
#include "scenegraph/callmethod_task.hpp"
#include "scenegraph/foreach_task.hpp"
#include "taskgraph/singlethread_scheduler.hpp"
#include "resource/orkmesh_loader.hpp"

int const WIDTH = 800;
int const HEIGHT = 600;

class main_window
	: public sdl_window
{
public:
	main_window();	
	void display();
	void reshape(int w, int h);

private:
	ptr<method> create_object_method(ptr<scene_node> n) const;
	ptr<method> create_camera_draw_method(ptr<scene_node> c) const;
	ptr<mesh_buffers> load_cube_mesh() const;

	GLuint _vao;
	scene_manager _scene;
	framebuffer & _fb;
};

main_window::main_window()
	: sdl_window(window::parameters().version(3, 3).size(WIDTH, HEIGHT).name("scene-graph test"))
	, _fb(framebuffer::default_fb())
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	ptr<scene_node> root = make_ptr<scene_node>();

	_scene.scheduler(make_ptr<singlethread_scheduler>());
	_scene.root(root);

	// camera
	ptr<scene_node> camera = make_ptr<scene_node>();
	camera->append_flag("camera");
	camera->assoc_method("draw", create_camera_draw_method(camera));
	root->append_child(camera);

	_scene.camera_to_screen(
		glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f));

	_scene.camera_node(camera);
	_scene.camera_method("draw");

	// cube
	ptr<scene_node> cube = make_ptr<scene_node>();
	cube->append_flag("object");
	ptr<mesh_buffers> cube_mesh = load_cube_mesh();
	cube->assoc_mesh("geometry", cube_mesh);
	cube->assoc_method("draw", create_object_method(cube));
	root->append_child(cube);

	_fb.depth_test(true);
}

void main_window::display()
{
	_fb.clear(true, true, false);

	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	_scene.camera_node()->local_to_parent(glm::inverse(V));

	_scene.update(0.0, 0.0);
	_scene.draw();

	sdl_window::display();
}

void main_window::reshape(int w, int h)
{
	_fb.depth_test(true);

	_scene.camera_to_screen(glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f));

	sdl_window::reshape(w, h);
}

ptr<method> main_window::create_object_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	std::vector<std::string> modules{std::string{"shader/cubes_transf.vs"}, std::string{"shader/cubes.fs"}};
	subtasks.push_back(make_ptr<program_task_factory>(modules));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	subtasks.push_back(make_ptr<draw_mesh_factory>("geometry"));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

ptr<method> main_window::create_camera_draw_method(ptr<scene_node> c) const
{
	std::vector<ptr<task_factory>> subtasks;
	std::vector<std::string> modules{std::string{"shader/cubes_transf.vs"}, std::string{"shader/cubes.fs"}};
	subtasks.push_back(make_ptr<program_task_factory>(modules));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	ptr<call_method_task_factory> draw_meth = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "object", draw_meth));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, c);
}

ptr<mesh_buffers> main_window::load_cube_mesh() const
{
	orkmesh_loader loader;
	return loader.load("meshes/cube.mesh");
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

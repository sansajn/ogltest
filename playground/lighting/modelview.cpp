/*! \file lighting.cpp
V okne zobrazi nasvieteny a rotujuci model. */
#include <vector>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ptr.h"
#include "render/framebuffer.h"
#include "render/meshbuffers.h"
#include "render/program.h"
#include "scenegraph/scene.hpp"
#include "scenegraph/scenenode.hpp"
#include "scenegraph/method.hpp"
#include "scenegraph/program_task.hpp"
#include "scenegraph/transforms_task.hpp"
#include "scenegraph/drawmesh.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/callmethod_task.hpp"
#include "scenegraph/foreach_task.hpp"
#include "taskgraph/singlethread_scheduler.hpp"
#include "resource/assimp_loader.h"
#include "ui/sdl_window.h"

int const WIDTH = 800;
int const HEIGHT = 600;
char const MODEL_FILE[] = "models/monkey.ply";
char const * PROGRAM_MODULES = "shaders/ambient.vs;shaders/ambient.fs";

class main_window : public sdl_window  // TODO: sdl okono nie je resizable
{
public:
	main_window();
	void display() override;
	void reshape(int w, int h) override;

private:
	ptr<method> create_model_draw_method(ptr<scene_node> n) const;
	ptr<method> create_camera_draw_method(ptr<scene_node> n) const;
	ptr<mesh_buffers> load_model_mesh() const;

	// light
	glm::vec4 _ambient;

	scene_manager _scene;
	ptr<scene_node> _model;
	framebuffer & _fb;
	GLuint _vao;
};  // main_window

glm::vec3 origin{0.0f, 0.0f, 0.0f};
glm::vec3 up{0.0f, 1.0f, 0.0f};
glm::vec3 campos{0.0f, 0.0f, 5.0f};

void main_window::display()
{
	_ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	_fb.clear(true, true);

	glm::mat4 V = glm::lookAt(campos, origin, up);
	_scene.camera_node()->local_to_parent(glm::inverse(V));

	static float theta = 0.0f;
	theta += 0.01f;

	glm::mat4 R = glm::rotate(glm::mat4(1), glm::degrees(theta), glm::vec3(up));
	_model->local_to_parent(R);

	_scene.update(0.0, 0.0);  // TODO: implement times
	_scene.draw();

	sdl_window::display();
}

main_window::main_window()
	: sdl_window(parameters().name("lighting").size(WIDTH, HEIGHT).debug(true))
	, _fb(framebuffer::default_fb())
{	
	glGenVertexArrays(1, &_vao);  // TODO: bez vertex-arrays to nejede
	glBindVertexArray(_vao);

	ptr<scene_node> root = make_ptr<scene_node>();

	_scene.scheduler(make_ptr<singlethread_scheduler>());
	_scene.root(root);
	
	// kamera
	ptr<scene_node> camera = make_ptr<scene_node>();
	camera->append_flag("camera");
	camera->assoc_method("draw", create_camera_draw_method(camera));
	root->append_child(camera);

	_scene.camera_to_screen(
		glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.1f, 100.0f));

	_scene.camera_node(camera);
	_scene.camera_method("draw");

	// model
	_model = make_ptr<scene_node>();
	_model->append_flag("object");
	ptr<mesh_buffers> model_mesh = load_model_mesh();
	_model->assoc_mesh("geometry", model_mesh);
	_model->assoc_method("draw", create_model_draw_method(_model));
	root->append_child(_model);
	
	_fb.depth_test(true);  // TODO: ak by sa po vytvorení okna zavolal reshape
}

void main_window::reshape(int w, int h)
{
	// TODO: defaultna implementacia nic nerobi, co je skoda (aspon nastavit glViewPort() by mohla
}

ptr<method> main_window::create_model_draw_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	subtasks.push_back(make_ptr<program_task_factory>(PROGRAM_MODULES));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	subtasks.push_back(make_ptr<draw_mesh_factory>("geometry"));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

ptr<method> main_window::create_camera_draw_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	subtasks.push_back(make_ptr<program_task_factory>(PROGRAM_MODULES));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	ptr<call_method_task_factory> _method_task =	make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "object", _method_task));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

ptr<mesh_buffers> main_window::load_model_mesh() const
{
	assimp_loader loader;  // TODO: nazov assimp zamaskuj, co tak common_mesh_loader ?
	return loader.load(MODEL_FILE);
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

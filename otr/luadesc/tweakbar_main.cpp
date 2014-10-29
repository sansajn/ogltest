#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include "core/ptr.hpp"
#include "scenegraph/scene.hpp"
#include "scenegraph/method.hpp"
#include "scenegraph/transforms_task.hpp"
#include "scenegraph/drawmesh.hpp"
#include "scenegraph/program_task.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/callmethod_task.hpp"
#include "scenegraph/foreach_task.hpp"
#include "taskgraph/singlethread_scheduler.hpp"
#include "render/meshbuffers.h"
#include "render/program.hpp"
#include "resource/assimp_loader.hpp"
#include "ui/glut_window.hpp"

#include "tweakbar_resource.hpp"
#include "tweakbar_manager.hpp"
#include "draw_tweakbar_task.hpp"

using std::cout;

int const WIDTH = 800;
int const HEIGHT = 600;

char const MODEL_FILE[] = "model/monkey.ply";
char const SCRIPT_FILE[] = "script/tweakbar.lua";
char const PROGRAM_MODULES[] = "shader/nolighting.vs;shader/nolighting.fs";

class app_window : public glut_window
{
public:
	typedef glut_window base;

	app_window();
	~app_window() {}

	void display() override;
	void reshape(int w, int h) override;
	bool mouse_motion(int x, int y) override;
	bool mouse_passive_motion(int x, int y) override;
	bool mouse_wheel(wheel b, int x, int y) override;
	bool mouse_click(button b, state s, int x, int y) override;
	bool key_typed(unsigned char c, int x, int y) override;
	bool key_released(unsigned char c, int x, int y) override;
	bool special_key(key k, int x, int y) override;
	bool special_key_released(key k, int x, int y) override;

private:
	ptr<method> create_model_draw_method(ptr<scene_node> n) const;
	ptr<method> create_camera_draw_method(ptr<scene_node> n) const;
	ptr<method> create_twbar_draw_method(ptr<scene_node> n) const;
	ptr<mesh_buffers> load_model_mesh() const;
	void tweakbar_init();

	ptr<tweakbar_manager> _ui;

	scene_manager _scene;
	ptr<scene_node> _model;
	framebuffer & _fb;
	GLuint _vao;
};

glm::vec3 origin{0.0f, 0.0f, 0.0f};
glm::vec3 up{0.0f, 1.0f, 0.0f};
glm::vec3 campos{0.0f, 0.0f, 5.0f};

app_window::app_window()
	: base(parameters().size(WIDTH, HEIGHT).name("AntTwekBar test"))
	, _fb(framebuffer::default_fb())
{
	glGenVertexArrays(1, &_vao);
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
	
	// tweakbar
	ptr<scene_node> tweakbar = make_ptr<scene_node>();
	tweakbar->append_flag("overlay");
	tweakbar->assoc_method("draw", create_twbar_draw_method(tweakbar));
	root->append_child(tweakbar);

	_fb.depth_test(true);

	tweakbar_init();
}

void app_window::tweakbar_init()
{
	std::vector<twbar_variable_desc> vars = read_tweakbar_as_lua(SCRIPT_FILE);

	ptr<tweakbar_handler> handler(new tweakbar_resource(vars));

	tweakbar_manager::bar_data data(handler, false, true, 'o');
	std::vector<tweakbar_manager::bar_data> datas;
	datas.push_back(data);

	_ui = make_ptr<tweakbar_manager>(datas);
}

void app_window::display()
{
	_fb.clear(true, true);

	glm::mat4 V = glm::lookAt(campos, origin, up);
	_scene.camera_node()->local_to_parent(glm::inverse(V));

	glBindVertexArray(_vao);

	_scene.update(0.0, 0.0);  // TODO: implement times
	_scene.draw();

	_ui->display();
	base::display();
}

void app_window::reshape(int w, int h)
{
	_ui->reshape(w, h);
	base::reshape(w, h);
}

bool app_window::mouse_motion(int x, int y)
{
	return _ui->mouse_motion(x, y);
}

bool app_window::mouse_passive_motion(int x, int y)
{
	return _ui->mouse_passive_motion(x, y);
}

bool app_window::mouse_wheel(wheel b, int x, int y)
{
	return _ui->mouse_wheel(b, x, y);
}

bool app_window::mouse_click(button b, state s, int x, int y)
{
	return _ui->mouse_click(b, s, x, y);
}

bool app_window::key_typed(unsigned char c, int x, int y)
{
	return _ui->key_typed(c, x, y);  // TODO: modifiers not handled
}

bool app_window::key_released(unsigned char c, int x, int y)
{
	return _ui->key_released(c, x, y);
}

bool app_window::special_key(key k, int x, int y)
{
	return _ui->special_key(k, x, y);
}

bool app_window::special_key_released(key k, int x, int y)
{
	return _ui->special_key_released(k, x, y);
}

ptr<method> app_window::create_model_draw_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	subtasks.push_back(make_ptr<program_task_factory>(PROGRAM_MODULES));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));
	subtasks.push_back(make_ptr<draw_mesh_factory>("geometry"));
	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

ptr<method> app_window::create_twbar_draw_method(ptr<scene_node> n) const
{
	ptr<draw_tweakbar_factory> fact = make_ptr<draw_tweakbar_factory>();
	return make_ptr<method>(fact, n);
}

ptr<method> app_window::create_camera_draw_method(ptr<scene_node> n) const
{
	std::vector<ptr<task_factory>> subtasks;
	subtasks.push_back(make_ptr<program_task_factory>(PROGRAM_MODULES));
	subtasks.push_back(make_ptr<transforms_task_factory>("local_to_screen"));

	// zavolaj draw pre vsetky "object"
	ptr<call_method_task_factory> object_draw_meth = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "object", object_draw_meth));

	// zavolaj draw pre vsetky "overlay"
	ptr<call_method_task_factory> overlay_draw_meth = make_ptr<call_method_task_factory>(task_factory::qualified_name("$o.draw"));
	subtasks.push_back(make_ptr<foreach_task_factory>("o", "overlay", overlay_draw_meth));

	ptr<sequence_task_factory> seq = make_ptr<sequence_task_factory>(subtasks);
	return make_ptr<method>(seq, n);
}

ptr<mesh_buffers> app_window::load_model_mesh() const
{
	assimp_loader loader;  // TODO: nazov assimp zamaskuj, co tak common_mesh_loader ?
	return loader.load(MODEL_FILE);
}


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

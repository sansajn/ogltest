// tweakbar test
#include <map>
#include <set>
#include <vector>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "luatools/luatools.hpp"
#include "luatools/error_output.hpp"
#include "core/ptr.hpp"
#include "render/framebuffer.hpp"
#include "taskgraph/singlethread_scheduler.hpp"
#include "scenegraph/scene.hpp"
#include "resource/resource_factory.hpp"
#include "resource/resource_manager.hpp"
#include "resource/lua_resource.hpp"
#include "resource/lua_resource_loader.hpp"
#include "resource/lua_scenegraph.hpp"
#include "resource/lua_tweakbar.hpp"
#include "ui/glut_window.hpp"
#include "ui/twbar/tweakbar_resource.hpp"
#include "ui/twbar/tweakbar_manager.hpp"
#include "ui/twbar/draw_tweakbar_task.hpp"

#include <pthread.h>

using namespace std;


resource_factory::type<lua_node_resource> node_type("node");
resource_factory::type<lua_foreach_factory_resource> foreach_factory_type("foreach");
resource_factory::type<lua_callmethod_factory_resource> callmethod_factory_type("callmethod");
resource_factory::type<lua_module_resource> module_type("module");
resource_factory::type<lua_mesh_resource> mesh_type("mesh");
resource_factory::type<lua_sequence_resource> sequence_type("sequence");
resource_factory::type<lua_setprogram_resource> setprogram_type("setprogram");
resource_factory::type<lua_transforms_resource> transforms_type("transforms");
resource_factory::type<lua_drawmesh_resource> drawmesh_type("drawmesh");
resource_factory::type<lua_program_resource> program_type("program");
resource_factory::type<lua_draw_tweakbar_resource> draw_tweakbar_type("drawtweakbar");
resource_factory::type<lua_tweakbar_manager_resource> tweakbar_manager_type("tweakbar_manager");
resource_factory::type<lua_tweakbar_resource> tweakbar_type("tweakbar");


class app : public glut_window
{
public:
	typedef glut_window base;

	app();
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
	ptr<scene_manager> _sceneman;
	ptr<tweakbar_manager> _ui;
	framebuffer & _fb;
};

app::app() : _fb(framebuffer::default_fb())
{
	auto loader = make_ptr<lua_resource_loader>("res/scenedesc.lua");
	loader->append_path("res");
	auto resman = make_ptr<resource_manager>(loader);

	_sceneman = make_ptr<scene_manager>();
	_sceneman->resources(resman);
	_sceneman->scheduler(make_ptr<singlethread_scheduler>());

	ptr<scene_node> scene = resman->load_resource<scene_node>("scene");
	_ui = resman->load_resource<tweakbar_manager>("twbar_man");

	_sceneman->root(scene);
	_sceneman->camera_node("camera");
	_sceneman->camera_method("draw");
}

void app::display(double t, double dt)
{
	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_sceneman->camera_node()->local_to_parent(glm::inverse(V));
	_fb.clear();
	_sceneman->update(t, dt);
	_sceneman->draw();
	_ui->display(t, dt);
	base::display(t, dt);
	// TODO: frame based log flush
}

void app::reshape(int w, int h)
{
	glm::mat4 P = glm::perspective(60.0f, float(w)/h, 0.1f, 100.f);
	_sceneman->camera_to_screen(P);

	_fb.depth_test(true);
	_ui->reshape(w, h);
	base::reshape(w, h);
	// TODO: idle
}

int main(int argc, char * argv[])
{
	app a;
	a.start();
	int i = pthread_getconcurrency();  // Inconsistency detected by ld.so: dl-version.c: 224: ...
	return 0;
}


bool app::mouse_motion(int x, int y)
{
	return _ui->mouse_motion(x, y);
}

bool app::mouse_passive_motion(int x, int y)
{
	return _ui->mouse_passive_motion(x, y);
}

bool app::mouse_wheel(wheel b, modifier m, int x, int y)
{
	return _ui->mouse_wheel(b, m, x, y);
}

bool app::mouse_click(button b, state s, modifier m, int x, int y)
{
	return _ui->mouse_click(b, s, m, x, y);
}

bool app::key_typed(unsigned char c, modifier m, int x, int y)
{
	return _ui->key_typed(c, m, x, y);
}

bool app::key_released(unsigned char c, modifier m, int x, int y)
{
	return _ui->key_released(c, m, x, y);
}

bool app::special_key(key k, modifier m, int x, int y)
{
	return _ui->special_key(k, m, x, y);
}

bool app::special_key_released(key k, modifier m, int x, int y)
{
	return _ui->special_key_released(k, m, x, y);
}


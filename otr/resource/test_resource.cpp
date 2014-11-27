// pokus o odstrnenie nutnosti ovodzovt zdroje od resource
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
#include "ui/glut_window.hpp"
#include "resource.hpp"
#include "resource_loader.hpp"
#include "resource_factory.hpp"
#include "resource_manager.hpp"
#include "lua_resource.hpp"
#include "lua_resource_loader.hpp"
#include "lua_scenegraph.hpp"

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


class app : public glut_window
{
public:
	typedef glut_window base;

	app();
	void display(double t, double dt) override;
	void reshape(int w, int h) override;

private:
	ptr<scene_manager> _sceneman;
	framebuffer & _fb;
};

app::app() : _fb(framebuffer::default_fb())
{
	auto loader = make_ptr<lua_resource_loader>("res/simple_scene.lua");
	loader->append_path("res");
	auto resman = make_ptr<resource_manager>(loader);

	_sceneman = make_ptr<scene_manager>();
	_sceneman->resources(resman);
	_sceneman->scheduler(make_ptr<singlethread_scheduler>());

	ptr<scene_node> scene = resman->load_resource<scene_node>("scene");

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
	base::display(t, dt);
	// TODO: frame based log flush
}

void app::reshape(int w, int h)
{
	glm::mat4 P = glm::perspective(60.0f, float(w)/h, 0.1f, 100.f);
	_sceneman->camera_to_screen(P);

	_fb.depth_test(true);
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

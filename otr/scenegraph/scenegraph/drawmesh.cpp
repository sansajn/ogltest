#include "scenegraph/drawmesh.hpp"
#include "core/logger.hpp"
#include "scenegraph/scene.hpp"

class draw_mesh_task	: public task
{
public:
	draw_mesh_task(ptr<mesh_buffers> m) : task(true, 0),_m(m) {}
	bool run();

private:
	ptr<mesh_buffers> _m;
};

ptr<task> draw_mesh_task_factory::create_task(ptr<scene_node> context)
{
	ptr<mesh_buffers> m;
	ptr<scene_node> t = _mesh_name.target_node(context);
	if (t)
		m = t->get_mesh(_mesh_name.name);
	else
		m = context->owner()->resources()->load_resource<mesh_buffers>(_mesh_name.name + ".mesh");

	if (!m)
		task_exception("SCENEGRAPH", "draw_mesh_task_factory: cannot find mesh '" + _mesh_name.target + "." + _mesh_name.name + "'");

	return std::make_shared<draw_mesh_task>(m);
}

bool draw_mesh_task::run()
{
	dlog("SCENEGRAPH") << "draw_mesh_task::run()";
	ptr<shader::program> prog = scene_manager::current_program();
	scene_manager::current_framebuffer()->draw(*prog, *_m);
	return true;
}

#include "drawmesh.h"
#include "scene.h"

bool draw_mesh_task::run()
{
	scene_manager::current_framebuffer()->draw(*scene_manager::current_program(),
		*_m, _m->mode());
	return true;
}

ptr<task> draw_mesh_factory::create_task(ptr<scene_node> n)
{
	ptr<mesh_buffers> m = n->get_mesh(_mesh_name);
	assert(m && "error: mesh not found");
	return std::make_shared<draw_mesh_task>(m);
}

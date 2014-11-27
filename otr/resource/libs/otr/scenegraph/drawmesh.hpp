#pragma once
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"
#include "render/meshbuffers.hpp"

class scene_node;

/*! Vytvara task zobrazujúci mriezku.
\ingroup render */
class draw_mesh_task_factory : public task_factory
{
public:
	draw_mesh_task_factory(qualified_name const & mesh_name) : _mesh_name(mesh_name) {}
	ptr<task> create_task(ptr<scene_node> context) override;

protected:
	draw_mesh_task_factory() {}
	void init(qualified_name const & mesh_name, int count);

private:
	qualified_name _mesh_name;  // odkazovanie nazvom umoznuje zmenu geometrie (bez zmeny factory)
};

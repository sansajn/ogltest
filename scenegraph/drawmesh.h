#pragma once
#include "ptr.h"
#include "task.h"
#include "meshbuffers.h"
#include "scenenode.h"


/*! Task zobrazujúci mesh. 
Zjednodušená verzia bez run-type schopností. */
class draw_mesh_task	: public task
{
public:
	draw_mesh_task(ptr<mesh_buffers> m) : _m(m) {}
	bool run();

private:
	ptr<mesh_buffers> _m;
};


class draw_mesh_factory : public task_factory
{
public:
	draw_mesh_factory(std::string const & mesh_name) : _mesh_name(mesh_name) {}
	ptr<task> create_task(ptr<scene_node> n);  //!< \param n uzol pre ktory task vytvaram

private:
	std::string _mesh_name;  // odkazovanie nazvom umoznuje zmenu geometrie (bez zmeny factory)
};

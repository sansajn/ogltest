#pragma once
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

/*! \ingroup xxx */
class draw_tweakbar_factory : public task_factory
{
public:
	draw_tweakbar_factory() {}
	ptr<task> create_task(ptr<scene_node> context) override;
};

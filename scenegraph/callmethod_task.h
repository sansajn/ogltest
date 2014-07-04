#pragma once
#include "task.h"

/*! A task to execute a method (for a target).
@ingroup scenegraph */
class call_method_task_factory : public task_factory
{
public:
	call_method_task_factory(qualified_name const & meth_name)
		: _methname(meth_name)
	{}

	ptr<task> create_task(ptr<scene_node> context);

private:
	qualified_name _methname;
};

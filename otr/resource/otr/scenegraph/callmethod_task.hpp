#pragma once
#include "taskgraph/task.hpp"

// TODO: premenuj na methodtask.[h/cpp]

/*! A task to execute a method (for a target).
@ingroup scenegraph */
class call_method_task_factory : public task_factory
{
public:
	call_method_task_factory(qualified_name const & meth_name) : _methname(meth_name) {}

	ptr<task> create_task(ptr<scene_node> context) override;

protected:
	call_method_task_factory() {}
	void init(qualified_name const & methname) {_methname = methname;}

private:
	qualified_name _methname;
};

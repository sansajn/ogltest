#pragma once
#include <vector>
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

/*! Umoznuje zoskupovat tasky.
@ingroup scenegraph */
class sequence_task_factory : public task_factory
{
public:
	sequence_task_factory(std::vector<ptr<task_factory>> const & subtasks)
		: _subtasks(subtasks)
	{}

	virtual ~sequence_task_factory() {}

	ptr<task> create_task(ptr<scene_node> context);

private:
	std::vector<ptr<task_factory>> _subtasks;
};

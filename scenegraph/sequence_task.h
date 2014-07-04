#pragma once
#include <vector>
#include "ptr.h"
#include "task.h"

/*! Umoznuje zoskupovat tasky. */
class sequence_task_factory : public task_factory
{
public:
	sequence_task_factory(std::vector<ptr<task_factory>> const & subtasks)
		: _subtasks(subtasks)
	{}

	ptr<task> create_task(ptr<scene_node> context);

private:
	std::vector<ptr<task_factory>> _subtasks;
};

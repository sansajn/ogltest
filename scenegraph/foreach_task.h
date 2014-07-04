#pragma once
#include <string>
#include "ptr.h"
#include "task.h"

class foreach_task_factory : public task_factory
{
public:
	foreach_task_factory(std::string const & var, std::string const & flag,
		ptr<task_factory> subtask) : _var(var), _flag(flag), _subtask(subtask)
	{}

	ptr<task> create_task(ptr<scene_node> context);

private:
	std::string _var;
	std::string _flag;
	ptr<task_factory> _subtask;
};

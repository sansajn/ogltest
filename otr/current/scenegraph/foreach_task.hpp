#pragma once
#include <string>
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

/*! @ingroup scenegraph */
class foreach_task_factory : public task_factory
{
public:
	foreach_task_factory(std::string const & var, std::string const & flag,	bool cull, bool parallel, ptr<task_factory> subtask)
		: _var(var), _flag(flag), _cull(cull), _parallel(parallel), _subtask(subtask)
	{}

	ptr<task> create_task(ptr<scene_node> context);

protected:
	foreach_task_factory() {}
	void init(std::string const & var, std::string const & flag, bool cull, bool parallel, ptr<task_factory> subtask);

private:
	std::string _var;
	std::string _flag;
	bool _cull;
	bool _parallel;
	ptr<task_factory> _subtask;
};

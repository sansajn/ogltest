#pragma once
#include <vector>
#include <string>
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"
#include "render/program.hpp"

/*! @ingroup scenegraph */
class program_task_factory : public task_factory
{
public:
	program_task_factory(std::vector<qualified_name> const & modules) : _modules(modules) {}
	ptr<task> create_task(ptr<scene_node>);

private:
	ptr<program> _p;
	std::vector<qualified_name> _modules;
};

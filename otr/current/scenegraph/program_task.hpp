#pragma once
#include <vector>
#include <string>
#include "core/ptr.h"
#include "taskgraph/task.hpp"
#include "render/program.h"

/*! @ingroup scenegraph */
class program_task_factory : public task_factory
{
public:
	program_task_factory(std::vector<std::string> const & module_names);
	ptr<task> create_task(ptr<scene_node>);

private:
	ptr<shader_program> _p;
};
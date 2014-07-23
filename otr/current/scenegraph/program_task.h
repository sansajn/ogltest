#pragma once
#include <vector>
#include <string>
#include "core/ptr.h"
#include "taskgraph/task.h"
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

namespace detail {

// TODO: premiestny do cpp
class program_task : public task
{
public:
	program_task(ptr<shader_program> p) : _p(p) {}
	bool run();

private:
	ptr<shader_program> _p;
};

}  // detail


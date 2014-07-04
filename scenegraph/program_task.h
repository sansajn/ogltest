#pragma once
#include <vector>
#include <string>
#include "ptr.h"
#include "task.h"
#include "program.h"

/*! @ingroup taskgraph */
class program_task_factory : public task_factory
{
public:
	program_task_factory(std::vector<std::string> const & module_names);
	ptr<task> create_task(ptr<scene_node>);

private:
	ptr<gl::program> _p;
};

namespace detail {

class program_task : public task
{
public:
	program_task(ptr<gl::program> p) : _p(p) {}
	bool run();

private:
	ptr<gl::program> _p;
};

}  // detail


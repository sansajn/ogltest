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
	program_task_factory(char const * modules);  //!< "module1;module2;..."
	program_task_factory(std::vector<std::string> const & modules);
	ptr<task> create_task(ptr<scene_node>);

private:
	ptr<shader_program> _p;
};

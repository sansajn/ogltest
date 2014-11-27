#pragma once
#include <vector>
#include <string>
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"
#include "render/program.hpp"

/*! @ingroup scenegraph */
class program_task_factory : public task_factory  // TODO: premenuj na setprogram
{
public:
	program_task_factory(std::vector<qualified_name> const & modules, bool set_uniforms)
		: _modules(modules), _uniforms(set_uniforms)
	{}

	ptr<task> create_task(ptr<scene_node>);

protected:
	program_task_factory() {}
	void init(std::vector<qualified_name> const & modules, bool set_uniforms);

private:
	std::vector<qualified_name> _modules;
	bool _uniforms;
};

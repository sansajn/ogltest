#pragma once
#include <memory>
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"
#include "render/program.hpp"
#include "render/uniform.hpp"

/*! @ingroup scenegraph */
class transforms_task_factory : public task_factory
{
public:
	transforms_task_factory(qualified_name const & module_name, char const * ltos, char const * wp);  // TODO: zoznam stringou nahrad niecim ako char_traits
	ptr<task> create_task(ptr<scene_node> context);

private:
	class transforms_task : public task
	{
	public:
		transforms_task(ptr<scene_node> context, transforms_task_factory * source)
			: task(true, 0), _context(context), _src(source)
		{}

		bool run() override;

	private:
		ptr<scene_node> _context;
		transforms_task_factory * _src;
	};

	void reload_uniforms(program & prog);

	program * _last_prog;
	ptr<uniform_matrix4f> _local_to_screen;
	ptr<uniform3f> _world_pos;
	qualified_name _module_name;
	ptr<module> _module;

	char const * _ltos;  // uniform names
	char const * _wp;
};

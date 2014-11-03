#pragma once
#include <memory>
#include "taskgraph/task.hpp"
#include "render/program.hpp"

/*! @ingroup scenegraph */
class transforms_task_factory
	: public task_factory, public std::enable_shared_from_this<transforms_task_factory>  // TODO: asi memory leak (prever shared_from_this())
{
public:
	transforms_task_factory(char const * ltos) : _ltos(ltos) {}
	ptr<task> create_task(ptr<scene_node> context);

private:
	class task_impl : public task
	{
	public:
		task_impl(ptr<scene_node> context, ptr<transforms_task_factory> source)
			: task(true, 0), _context(context), _src(source)
		{}

		bool run() override;

	private:
		ptr<scene_node> _context;
		ptr<transforms_task_factory> _src;
	};  // task_impl

	void reload_uniforms(program & prog);

	ptr<program> _last_prog;
	ptr<uniform_matrix4f> _local_to_screen;

	char const * _ltos;  // uniform names
};

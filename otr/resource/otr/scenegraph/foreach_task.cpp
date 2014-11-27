#include "scenegraph/foreach_task.hpp"
#include "scenegraph/scene.hpp"
#include "taskgraph/taskgraph.hpp"

ptr<task> foreach_task_factory::create_task(ptr<scene_node> context)
{
	// TODO: implement dependencies
	ptr<task_graph> result = make_ptr<task_graph>();

	scene_manager * man = context->owner();
	assert(man && "owner already deleted");
	if (!man)
		return result;

	for (auto const & n : man->nodes(_flag))
	{
		man->node_variable(_var, n.second);
		result->append_task(_subtask->create_task(context));
	}

	return result;
}

void foreach_task_factory::init(std::string const & var, std::string const & flag, bool cull, bool parallel, ptr<task_factory> subtask)
{
	_var = var;
	_flag = flag;
	_cull = cull;
	_parallel = parallel;
	_subtask = subtask;
}

#include "foreach_task.h"
#include "scenegraph/scene.h"
#include "taskgraph/taskgraph.h"

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

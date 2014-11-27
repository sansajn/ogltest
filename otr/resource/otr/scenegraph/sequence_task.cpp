#include "scenegraph/sequence_task.hpp"
#include "taskgraph/taskgraph.hpp"

ptr<task> sequence_task_factory::create_task(ptr<scene_node> context)
{
	if (_subtasks.size() == 1)
		return _subtasks[0]->create_task(context);

	ptr<task_graph> result = make_ptr<task_graph>();
	ptr<task> prev = nullptr;
	for (ptr<task_factory> subtask : _subtasks)
	{
		ptr<task> t = subtask->create_task(context);
		if (t)
		{
			result->append_task(t);
			if (prev)
				result->append_dependency(prev, t);
			prev = t;
		}
	}

	return result;
}

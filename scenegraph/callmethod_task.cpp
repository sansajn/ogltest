#include <exception>
#include "callmethod_task.h"
#include "ptr.h"
#include "method.h"
#include "scenenode.h"
#include "taskgraph.h"

ptr<task> call_method_task_factory::create_task(ptr<scene_node> context)
{
	ptr<scene_node> target = _methname.get_target(context);
	if (target)
	{
		ptr<method> m = target->get_method(_methname.name);

		assert(m && "method not found");

		if (m)
			return m->create_task();
		else
			return make_ptr<task_graph>();
	}

	throw std::exception();  // TODO: specify exception
}
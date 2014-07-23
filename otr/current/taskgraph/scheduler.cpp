#include "scheduler.h"
#include "taskgraph/taskgraph.h"

void singlethread_scheduler::schedule(ptr<task> t)
{
	append_flattened_task(t);
}

void singlethread_scheduler::run(ptr<task> t)
{
	schedule(t);
	for (auto u : _immediate_tasks)
		u->run();
	_immediate_tasks.clear();  // all tasks done
}

void singlethread_scheduler::append_flattened_task(ptr<task> t)
{
	// TODO: task dependencies isn't implementation
	ptr<task_graph> g = std::dynamic_pointer_cast<task_graph>(t);
	if (!g)  // primitive task
		_immediate_tasks.push_back(t);
	else
	{
		for (auto u : g->all_tasks())
			append_flattened_task(u);
	}
}

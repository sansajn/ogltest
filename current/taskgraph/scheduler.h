#pragma once
#include <vector>
#include "core/ptr.h"
#include "taskgraph/task.h"

/*! @ingroup taskgraph */
class task_scheduler
{
public:
	virtual ~task_scheduler() {}
	virtual void schedule(ptr<task> t) {}
	virtual void run(ptr<task> t) = 0;
};

/*! @ingroup taskgraph */
class singlethread_scheduler : public task_scheduler
{
public:
	void schedule(ptr<task> t);
	void run(ptr<task> t);

private:
	void append_flattened_task(ptr<task> t);

	std::vector<ptr<task>> _immediate_tasks;
};

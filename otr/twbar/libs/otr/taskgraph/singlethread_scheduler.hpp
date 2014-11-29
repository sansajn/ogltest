#pragma once
#include "taskgraph/task.hpp"
#include "taskgraph/scheduler.hpp"

/*! Implementuje jednovlaknovy planovac taskou, bez podpory prefetch.
@ingroup taskgraph */
class singlethread_scheduler : public task_scheduler
{
public:
	void run(ptr<task> t) override;
	void schedule(ptr<task> t) override;
};

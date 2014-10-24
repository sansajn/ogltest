#pragma once
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

/*! @ingroup taskgraph */
class task_scheduler
{
public:
	virtual ~task_scheduler() {}
	virtual void run(ptr<task> t) = 0;
	virtual void schedule(ptr<task> t) {}
	virtual bool supports_prefetch(bool gputasks) {return false;}
};  // task_scheduler

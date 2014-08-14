#pragma once
#include <map>
#include <list>
#include "core/ptr.h"
#include "taskgraph/task.hpp"

/*! @ingroup taskgraph */
class task_graph : public task
{
public:
	using graph_type = std::map<ptr<task>, std::list<ptr<task>>>;

	task_graph() : task(false, 0) {}  // TODO: ako inicializovat task ?
	task_graph(ptr<task> t) : task(false, 0) {append_task(t);}

	void append_task(ptr<task> t) {_tasks[t];}
	void remove_task(ptr<task> t) {_tasks.erase(t);}
	void append_dependency(ptr<task> s, ptr<task> t);
	void remove_dependency(ptr<task> s, ptr<task> t);
	bool empty() const {return _tasks.empty();}
	graph_type const & graph() const {return _tasks;}

	bool run() {return true;}

private:
	graph_type _tasks;
};  // task_graph

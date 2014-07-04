// TODO: task, tashgraph a task_factory by mohli byt v jednom súbore
#pragma once
#include <map>
#include <set>
#include <vector>
#include <boost/range.hpp>
#include "ptr.h"
#include "task.h"

// TODO: plna podpora task grafu

/*! @ingroup taskgraph
\note implementuje iba jednoduchy fifo graf */
class task_graph : public task
{
public:
//	typedef boost::iterator_range<std::set<ptr<task>>::iterator> task_range;
	typedef boost::iterator_range<std::vector<ptr<task>>::iterator> task_range;

	task_graph() {}
	task_graph(ptr<task> t) {append_task(t);}
	task_range all_tasks() {return boost::make_iterator_range(_tasks);}
//	task_range first_tasks() {return boost::make_iterator_range(_first_tasks);}
//	task_range last_tasks() {return boost::make_iterator_range(_last_tasks);}
//	task_range dependencies(ptr<task> t);
//	task_range inverse_dependencies(ptr<task> t);
//	void append_task(ptr<task> t);
	void append_task(ptr<task> t) {_tasks.push_back(t);}
//	void remove_task(ptr<task> t);
	
//	//! src je zavisle na dst
//	void append_dependency(ptr<task> src, ptr<task> dst);
	void append_dependency(ptr<task> src, ptr<task> dst) {}

//	void remove_dependency(ptr<task> src, ptr<task> dst);

//	void remove_and_get_dependencies(
//		ptr<task> src, std::set<ptr<task>> & deleted_dependencies);

//	void clear_dependencies();
	bool empty() const {return _tasks.empty();}

	bool run() {return true;}  // TODO: docastne riesenie mechanizmu taskou

private:
//	typedef std::map<ptr<task>, std::set<ptr<task>>> dependency_map;

//	task_range find_dependencies(dependency_map & src, ptr<task> t);

//	std::set<ptr<task>> _tasks;
	std::vector<ptr<task>> _tasks;
//	std::set<ptr<task>> _first_tasks;
//	std::set<ptr<task>> _last_tasks;
//	std::set<ptr<task>> _flattened_first_tasks;
//	std::set<ptr<task>> _flattened_last_tasks;
//	dependency_map _dependencies;
//	dependency_map _inverse_dependencies;
};

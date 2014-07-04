#include "taskgraph.h"

//task_graph::task_range task_graph::dependencies(ptr<task> t)
//{
//	return find_dependencies(_dependencies, t);
//}

//task_graph::task_range task_graph::inverse_dependencies(ptr<task> t)
//{
//	return find_dependencies(_inverse_dependencies, t);
//}

//void task_graph::append_task(ptr<task> t)
//{
//	if (_tasks.find(t) != _tasks.end())
//		return;
//	_tasks.insert(t);
//	_first_tasks.insert(t);
//	_last_tasks.insert(t);
//}

//void task_graph::remove_task(ptr<task> t)
//{
//	if (_tasks.find(t) == _tasks.end())
//		return;
//	_tasks.erase(t);
//	_first_tasks.erase(t);
//	_last_tasks.erase(t);
//	assert(_dependencies.find(t) == _dependencies.end());
//	assert(_inverse_dependencies.find(t) == _inverse_dependencies.end());
//}

////! src je zavisle na dst
//void task_graph::append_dependency(ptr<task> src, ptr<task> dst)
//{
//	_first_tasks.erase(src);
//	_last_tasks.erase(dst);
//	_dependencies[src].insert(dst);
//	_inverse_dependencies[dst].insert(src);
//}

//void task_graph::remove_dependency(ptr<task> src, ptr<task> dst)
//{
//	_dependencies[src].erase(dst);
//	if (_dependencies[src].empty())
//	{
//		_dependencies.erase(_dependencies.find(src));
//		_first_tasks.insert(src);
//	}

//	_inverse_dependencies[dst].erase(src);
//	if (_inverse_dependencies[dst].empty())
//	{
//		_inverse_dependencies.erase(_inverse_dependencies.find(dst));
//		_last_tasks.insert(dst);
//	}
//}

//void task_graph::clear_dependencies()
//{
//	_first_tasks.insert(_tasks.begin(), _tasks.end());
//	_last_tasks.insert(_tasks.begin(), _tasks.end());
//	_dependencies.clear();
//	_inverse_dependencies.clear();
//}

//task_graph::task_range task_graph::find_dependencies(
//	dependency_map & src, ptr<task> t)
//{
//	auto it = src.find(t);
//	if (it != src.end())
//		return boost::make_iterator_range(it->second);
//	else
//		return task_range();
//}

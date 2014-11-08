#include "taskgraph/taskgraph.hpp"
#include <algorithm>

void task_graph::append_dependency(ptr<task> s, ptr<task> t)
{
	_tasks[s].push_back(t);
}

void task_graph::remove_dependency(ptr<task> s, ptr<task> t)
{
	auto & edges = _tasks[s];
	auto it = std::find(edges.begin(), edges.end(), t);
	if (it != edges.end())
		edges.erase(it);
}

/*
task_graph::flatten_task_range::flatten_task_range(task_set & tasks)
	: _it(tasks.begin()), _end(tasks.end())
{
	_done = (_it == _end);
}

bool task_graph::flatten_task_range::next()
{
	++_it;
	if (_it != _end)
	{
		ptr<task_graph> g = std::dynamic_pointer_cast<task_graph>(*_it);
		if (g)
		{
			_itstack.push(make_pair(_it, _end));
			if (g->_tasks.empty())
				return next();
			_it = g->_tasks.begin();
			_end = g->_tasks.end();
		}
		return true;
	}
	else
	{
		if (_itstack.empty())
			return false;
		_it = _itstack.top().first;
		_end = _itstack.top().second;
		_itstack.pop();
		return next();
	}
	return false;
}
*/

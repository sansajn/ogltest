#include "singlethread_scheduler.hpp"
#include <deque>
#include <vector>
#include <cassert>
#include "taskgraph/map_as_graph.hpp"  // musí byť pred topological_sort.hpp
#include <boost/graph/topological_sort.hpp>
#include "taskgraph/taskgraph.hpp"
#include <GL/glew.h>

// TODO: uprav pre použitie s taskom
static std::vector<ptr<task>> singlethread_sort(task_graph const & g);

void singlethread_scheduler::run(ptr<task> t)
{
	std::vector<ptr<task>> immediate_tasks;

	ptr<task_graph> g = std::dynamic_pointer_cast<task_graph>(t);
	if (g)
		immediate_tasks = singlethread_sort(t);
	else
		immediate_tasks.push_back(t);

	for (auto t : immediate_tasks)
		if (!t->done())
		{
			t->run();
			assert(glGetError() == GL_NO_ERROR);
		}
}

void singlethread_scheduler::schedule(ptr<task> t)
{
	assert(true && "not yet implemented");
}

std::vector<ptr<task>> singlethread_sort(task_graph const & g)
{
	using namespace boost;
	using color_map_t = std::map<graph_traits<task_graph::graph_type>::vertex_descriptor, size_t>;

	color_map_t cmap;
	associative_property_map<color_map_t> colors(cmap);
	std::deque<ptr<task>> ordered;
	topological_sort(g.graph(), std::front_inserter(ordered), color_map(colors));

	std::vector<ptr<task>> tasks;

	for (auto t : ordered)
	{
		ptr<task_graph> tg = std::dynamic_pointer_cast<task_graph>(t);
		if (tg)  // taskgraph
		{
			std::vector<ptr<task>> subtasks = singlethread_sort(*tg);
			std::copy(subtasks.begin(), subtasks.end(), std::back_inserter(tasks));
		}
		else  // primitive task
			tasks.push_back(t);
	}

	return tasks;
}


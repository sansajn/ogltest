// riesenie zavyslosti taskou pre singlethread (pomocou topological_sort())
#include <map>
#include <list>
#include <deque>
#include <memory>
#include <iterator>
#include <iostream>
#include "map_as_graph.hpp"  // ako poriesim poziadavku na poradie headerov
#include <boost/graph/topological_sort.hpp>

template <typename T>
using ptr = std::shared_ptr<T>;

template <typename T, typename ... Args>
ptr<T> make_ptr(Args && ... args) {return std::make_shared<T>(args ...);}

struct task
{
	task(unsigned id) : id(id) {}
	virtual ~task() {}
	unsigned id;
};  // task

class taskgraph : public task
{
public:
	using graph_type = std::map<ptr<task>, std::list<ptr<task>>>;

	taskgraph() : task(0) {}
	void append_task(ptr<task> t) {_tasks[t];}
	void dependency(ptr<task> s, ptr<task> t) {_tasks[s].push_back(t);}
	graph_type const & graph() const {return _tasks;}

private:
	graph_type _tasks;
};  // taskgraph

std::vector<ptr<task>> singlethread_sort(taskgraph const & g)
{
	using namespace boost;
	using color_map_t = std::map<graph_traits<taskgraph::graph_type>::vertex_descriptor, size_t>;

	color_map_t cmap;
	associative_property_map<color_map_t> colors(cmap);
	std::deque<ptr<task>> ordered;
	topological_sort(g.graph(), std::front_inserter(ordered), color_map(colors));

	std::vector<ptr<task>> tasks;

	for (auto t : ordered)
	{
		ptr<taskgraph> tg = std::dynamic_pointer_cast<taskgraph>(t);
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

void test_singlethread_sort();
void test_pure_graph();

int main(int argc, char * argv[])
{
	test_singlethread_sort();
	return 0;
}

void test_singlethread_sort()
{
	using namespace boost;

	ptr<task> tasks[5];
	for (int i = 0; i < 5; ++i)
		tasks[i] = make_ptr<task>(i+1);

	taskgraph g;
	for (auto t : tasks)
		g.append_task(t);
	g.dependency(tasks[0], tasks[3]);
	g.dependency(tasks[1], tasks[4]);

	ptr<taskgraph> h(new taskgraph);
	ptr<task> t6(new task(7));
	ptr<task> t7(new task(8));
	ptr<task> t8(new task(9));
	h->append_task(t6);
	h->append_task(t7);
	h->append_task(t8);
	h->dependency(t7, t6);
	g.append_task(h);

	std::vector<ptr<task>> ordered = singlethread_sort(g);

	for (auto t : ordered)
		std::cout << t->id << ", ";
	std::cout << "\n";
}

void test_pure_graph()
{
	using namespace boost;

	ptr<task> tasks[5];
	for (int i = 0; i < 5; ++i)
		tasks[i] = make_ptr<task>(i);

	using G = std::map<ptr<task>, std::list<ptr<task>>>;
	G g;

	for (auto t : tasks)  // vertices
		g[t];

	add_edge(tasks[1], tasks[4], g);

	using vertex_descriptor = graph_traits<G>::vertex_descriptor;
	using color_map_t = std::map<vertex_descriptor, size_t>;
	color_map_t cmap;
	associative_property_map<color_map_t> colors(cmap);
	std::deque<ptr<task>> topo_order;

	topological_sort(g, std::front_inserter(topo_order), color_map(colors));

	for (auto t : topo_order)
		std::cout << t->id << ", ";
	std::cout << "\n";
}

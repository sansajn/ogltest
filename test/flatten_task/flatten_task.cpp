/* umožnuje iterovať tásky v task-graphe */
#include <set>
#include <stack>
#include <memory>
#include <utility>
#include <iostream>

using std::cout;
using std::make_pair;

template <typename T>
using ptr = std::shared_ptr<T>;

template <typename T, typename ... Args>
ptr<T> make_ptr(Args && ... args) {return std::make_shared<T>(args ...);}


struct task 
{	
	task(int n = 0) : id(n) {}
	virtual ~task() {}
	int id;
};  // task

class task_graph : public task
{	
public:
	typedef std::set<ptr<task>> task_set;

	class flatten_task_range
	{
	public:
		flatten_task_range(task_set & tasks);
		void operator++() {_done = !next();}
		ptr<task> operator*() {return *_it;}
		operator bool() const {return !_done;}

	private:
		bool next();

		task_set::iterator _it, _end;
		std::stack<std::pair<task_set::iterator, task_set::iterator>> _itstack;
		bool _done;
	};  // flatten_task_range

	void append_task(ptr<task> t) {_tasks.insert(t);}
	flatten_task_range all_flattened_tasks() {return flatten_task_range(_tasks);}

private:
	task_set _tasks;
};  // task_graph


int main(int argc, char * argv[])
{
	task_graph g;
	for (int i = 0; i < 3; ++i)
		g.append_task(make_ptr<task>(i));

	ptr<task_graph> h(make_ptr<task_graph>());
	for (int i = 3; i < 6; ++i)
		h->append_task(make_ptr<task>(i));
	g.append_task(h);

	for (int i = 6; i < 9; ++i)
		g.append_task(make_ptr<task>(i));

	ptr<task_graph> s(make_ptr<task_graph>());
	for (int i = 9; i < 12; ++i)
		s->append_task(make_ptr<task>(i));
	h->append_task(s);

	for (auto r = g.all_flattened_tasks(); r; ++r)
		cout << (*r)->id << ", ";
	cout << "\n";

	return 0;
}


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


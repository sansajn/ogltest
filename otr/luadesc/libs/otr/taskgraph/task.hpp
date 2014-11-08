#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "core/ptr.hpp"

class scene_node;  // fwd
class task_listener;

struct task_exception : public std::runtime_error
{
	task_exception(std::string const & msg) : task_exception("task_exception", msg) {}
	task_exception(std::string const & topic, std::string const & msg);
};

/*! @ingroup taskgraph */
class task
{
public:
	enum class reason
	{
		DEPENDENCY_CHANGED,
		DATA_CHANGED,
		DATA_NEEDED
	};

	task(bool gpu_task, unsigned deadline);
	virtual ~task() {}

	virtual bool run() = 0;	
	virtual bool done() const {return _done;}

	//! \param t completion date
	virtual void set_done(bool done, unsigned t, reason r = reason::DATA_NEEDED);

	bool gpu_task() const {return _gputask;}
	unsigned deadline() const {return _deadline;}
	void deadline(unsigned d) {_deadline = d;}
	unsigned completion_date() const {return _completion_date;}
	void append_listener(task_listener * l);
	void remove_listener(task_listener * l);

private:
	bool _done;
	bool _gputask;
	unsigned _deadline;
	unsigned _completion_date;
	std::vector<task_listener *> _listeners;
};  // task

/*! @ingroup taskgraph */
class task_factory
{
public:
	struct qualified_name
	{
		std::string target;
		std::string name;

		qualified_name(std::string const & n);  //!< \param n nazov v tvare '<target>.<name>'.
		ptr<scene_node> target_node(ptr<scene_node> context) const;
	};

	virtual ~task_factory() {}

	/*! \param context #scene_node ktoremu pati metoda vytvarajuca task. */
	virtual ptr<task> create_task(ptr<scene_node> context) = 0;
};

class task_listener
{
public:
	// TODO: task ma byt ptr<task>
	virtual void task_state_changed(task * t, bool done, task::reason r) = 0;
};


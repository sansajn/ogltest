#include "taskgraph/task.hpp"
#include "core/logger.hpp"
#include "scenegraph/scene.hpp"


task_exception::task_exception(std::string const & topic, std::string const & msg)
	: std::runtime_error(msg)
{
	error_log(topic, msg);
}

task::task(bool gputask, unsigned deadline)
	: _done(false), _gputask(gputask), _deadline(deadline), _completion_date(0)
{}

void task::set_done(bool done, unsigned t, reason r)
{
	if (_done == done)
		return;  // nothing changed

	_done = done;
	if (_done || r != reason::DEPENDENCY_CHANGED)
		_completion_date = t;

	for (auto & l : _listeners)
		l->task_state_changed(this, done, r);
}

void task::append_listener(task_listener * l)
{
	_listeners.push_back(l);
}

void task::remove_listener(task_listener * l)
{
	auto it = std::find(_listeners.begin(), _listeners.end(), l);
	if (it != _listeners.end())
		_listeners.erase(it);
}

task_factory::qualified_name::qualified_name(std::string const & n)
{
	auto dotpos = n.find('.');
	if (dotpos != std::string::npos)
	{
		target = n.substr(0, dotpos);
		name = n.substr(dotpos+1);
	}
	else
		name = n;
}

ptr<scene_node> task_factory::qualified_name::target_node(ptr<scene_node> context) const
{
	if (target.empty())
		return nullptr;
	else if (target == "this")
		return context;
	else if (target[0] == '$')
		return context->owner()->node_variable(target.substr(1));
	else
	{
		auto nodes = context->owner()->nodes(target);
		return !nodes.empty() ? nodes.begin()->second : nullptr;
	}
}

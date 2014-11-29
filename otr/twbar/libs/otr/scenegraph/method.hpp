#pragma once
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

class scene_node;

/*! Skrz method sa v scene_manager::draw() vytvaraju tasky pre uzol kameri.
\ingroup scenegraph */
class method
{
public:
	method(ptr<task_factory> factory) : _factory(factory), _enabled(true) {}
	void enable(bool b) {_enabled = b;}
	bool enabled() const {return _enabled;}
	ptr<task> create_task(ptr<scene_node> context) const {return _factory->create_task(context);}
	ptr<task_factory> get_factory() const {return _factory;}

private:
	ptr<task_factory> _factory;
	bool _enabled;
};

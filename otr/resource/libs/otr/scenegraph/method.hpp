#pragma once
#include "core/ptr.hpp"
#include "taskgraph/task.hpp"

class scene_node;

/*! Skrz method sa v scene_manager::draw() vytvaraju tasky pre uzol kameri.
\ingroup scenegraph */
class method
{
public:
	method(ptr<task_factory> factory) : _factory(factory) {}
	~method() {}
	ptr<task> create_task() const {return _factory->create_task(_owner);}
	ptr<task_factory> get_factory() const {return _factory;}

private:
	ptr<task_factory> _factory;
	ptr<scene_node> _owner;

	friend class scene_node;  //!< nastavuje _owner
};

#pragma once
#include "ptr.h"
#include "task.h"

class scene_node;  // fwd

/*! Statická implementácia (run-time-ovho) ork::method. */
class method
{
public:
	method(ptr<task_factory> factory, std::weak_ptr<scene_node> owner) : _factory(factory), _owner(owner) {}
	~method() {}
	std::weak_ptr<scene_node> owner() const {return _owner;}
	ptr<task> create_task() const {return _factory->create_task(_owner.lock());}
	ptr<task_factory> get_factory() const {return _factory;}

private:
	ptr<task_factory> _factory;
	std::weak_ptr<scene_node> _owner;
};

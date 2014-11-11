#pragma once
#include <set>
#include <map>
#include <vector>
#include <string>
#include "core/ptr.hpp"
#include "resource.hpp"

class resource_manager;

class task_factory
{
public:
	task_factory() {}
	virtual ~task_factory() {}
};

class foreach_factory : public task_factory
{
public:
	foreach_factory(std::string var, std::string flag, bool cull, bool parallel, ptr<task_factory> subtask)
		: _var(var), _flag(flag), _cull(cull), _parallel(parallel), _sub(subtask)
	{}

protected:
	foreach_factory() {}

	void init(std::string var, std::string flag, bool cull, bool parallel, ptr<task_factory> subtask)
	{
		_var = var;
		_flag = flag;
		_cull = cull;
		_parallel = parallel;
		_sub = subtask;
	}

private:
	std::string _var;
	std::string _flag;
	bool _cull;
	bool _parallel;
	ptr<task_factory> _sub;
};

class callmethod_factory : public task_factory
{
public:
	callmethod_factory(std::string methname) : _methname(methname) {}

protected:
	callmethod_factory() {}
	void init(std::string methname)	{_methname = methname;}

private:
	std::string _methname;
};

class method
{
public:
	method(ptr<task_factory> t) : _factory(t), _enabled(true) {}
	void enable(bool b) {_enabled = b;}

private:
	ptr<task_factory> _factory;
	bool _enabled;
};

class node
{
public:
	void append_child(ptr<node> n) {_children.push_back(n);}
	void append_flag(std::string const & f) {_flags.insert(f);}
	void assoc_method(std::string const & name, ptr<method> m) {_methods[name] = m;}

protected:
	node() {}

private:
	std::vector<ptr<node>> _children;
	std::set<std::string> _flags;
	std::map<std::string, ptr<method>> _methods;
};


class lua_node_resource : public resource_template<node>
{
public:
	lua_node_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_foreach_factory_resource : public resource_template<foreach_factory>
{
public:
	lua_foreach_factory_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_callmethod_factory_resource : public resource_template<callmethod_factory>
{
public:
	lua_callmethod_factory_resource(resource_descriptor * desc, resource_manager * resman);
};

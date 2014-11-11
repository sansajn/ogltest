#pragma once
#include <map>
#include <string>
#include "core/ptr.hpp"
#include "resource_loader.hpp"

class resource_manager
{
public:
	resource_manager(ptr<resource_loader> loader) : _loader(loader) {}

	ptr<resource> load_resource(std::string const & name);

	template <typename R>
	ptr<R> load_resource(std::string const & name) {return std::dynamic_pointer_cast<R>(load_resource(name));}

private:
	void insert_resource(std::string const & name, ptr<resource> r);

	std::map<std::string, ptr<resource>> _resources;
	ptr<resource_loader> _loader;
};

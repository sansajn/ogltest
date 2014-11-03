#include "resource_manager.hpp"
#include <cassert>

ptr<resource> resource_manager::load_resource(std::string const & name)
{
	auto it = _resources.find(name);
	if (it != _resources.end())
		return it->second;

	assert(it == _resources.end() && "resource not found");
	throw std::exception();  // TODO: specify
}

void resource_manager::insert_resource(std::string const & name, ptr<resource> r)
{
	auto result = _resources.insert(std::make_pair(name, r));
	assert(result.second && "resource already in manager");
}

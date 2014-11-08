#include "resource_manager.hpp"

ptr<resource> resource_manager::load_resource(std::string const & name)
{
	auto it = _resources.find(name);
	if (it != _resources.end())
		return it->second;

	assert(it == _resources.end() && "resource not found");
	throw std::exception();  // TODO: specify (resource not found)
}

void resource_manager::insert_resource(std::string const & name, ptr<resource> r)
{
	auto result = _resources.insert(std::make_pair(name, r));
	assert(result.second && "resource with this name already in a manager");
	if (!result.second)
		throw std::exception();  // TODO: specify (already in a manager)
}

std::string const & resource_manager::find_key(ptr<resource> r)
{
	for (auto const & kv : _resources)
		if (kv.second == r)
			return kv.first;
	throw std::exception();  // TODO: not found in manager
}

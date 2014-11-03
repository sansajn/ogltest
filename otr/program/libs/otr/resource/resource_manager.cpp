#include "resource_manager.hpp"

boost::any & resource_manager::load_resource(std::string const & name)
{
	auto it = _resources.find(name);
	if (it != _resources.end())
		return it->second;

	assert(it == _resources.end() && "resource not found");
	throw std::exception();  // TODO: specify
}

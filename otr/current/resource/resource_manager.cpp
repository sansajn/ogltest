#include "resource_manager.hpp"
#include <cassert>
#include "resource/resource_factory.hpp"

ptr<resource> resource_manager::load_resource(std::string const & name)
{
	// najprv sa pokus o hladanie v uz nacitanych zdrojoch
	auto it = _resources.find(name);
	if (it != _resources.end())
		return it->second;

	// zdroj nenajdeni, vytvor ho
	ptr<resource> r = _loader->create(name, *this);
	if (r)
		insert_resource(name, r);

	assert(r && "unable to create a resource");

	return r;
}

std::string resource_manager::find_key(ptr<resource> r) const
{
	for (auto kv : _resources)
	{
		if (r == kv.second)
			return kv.first;
	}
	assert(false && "resource not found");
	throw std::exception();
}

void resource_manager::insert_resource(std::string const & name, ptr<resource> r)
{
	auto result = _resources.insert(make_pair(name, r));
	assert(result.second && "resource already in a resource-manager");
}

#include "resource_manager.hpp"
#include <cassert>

ptr<resource> resource_manager::load_resource(std::string const & name)
{
	// najprv sa pokus o hladanie v uz nacitanych zdrojoch
	auto it = _resources.find(name);
	if (it != _resources.end())
		return it->second;

	// nenajdene, vytvor zdroj
	auto r = ptr<resource>(_loader->create(name, this));
	if (r)
		insert_resource(name, r);

	assert(r && "unable to create a resource");

	return r;
}

void resource_manager::insert_resource(std::string const & name, ptr<resource> r)
{
	auto result = _resources.insert(make_pair(name, r));
	assert(result.second && "resource already in a resource-manager");
}

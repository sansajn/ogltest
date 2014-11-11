#include "resource_factory.hpp"
#include <cassert>

resource_factory & resource_factory::ref()
{
	static resource_factory rf;
	return rf;
}

void resource_factory::append_type(std::string const & name, create_func f)
{
	auto res = _creators.insert(make_pair(name, f));
	assert(res.second && "resource with the same name already exists");
}

ptr<resource> resource_factory::create(resource_descriptor * desc, resource_manager * resman)
{
	auto it = _creators.find(desc->type);
	if (it != _creators.end())
		return it->second(desc, resman);
	else
		assert(false && "unknown resource type");

	return nullptr;
}

#pragma once
#include <string>

struct resource
{
	virtual ~resource() {}
};

struct resource_descriptor
{
	std::string type;  //!< node, callmethod, foreach, ...
	resource_descriptor(std::string const & type) : type(type) {}
	virtual ~resource_descriptor() {}
};

template <typename T>
class resource_template : public T, public resource
{};

#pragma once
#include "core/ptr.hpp"
#include "resource.hpp"

class resource_manager;

class resource_loader
{
public:
	virtual ~resource_loader() {}
	virtual ptr<resource> create(std::string const & name, resource_manager * resman) = 0;
};

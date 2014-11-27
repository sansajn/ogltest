#pragma once
#include "core/ptr.hpp"
#include "resource.hpp"

class resource_manager;


/*! Pripravuje descriptor pre factory (nájde požadovaný zdroj, prečíta binárne dáta).
\saa resource_descriptor, resource_manager, resource_factory */
class resource_loader
{
public:
	virtual ~resource_loader() {}
	virtual ptr<resource> create(std::string const & name, resource_manager & resman) = 0;
};

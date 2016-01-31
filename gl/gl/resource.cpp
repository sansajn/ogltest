#include "resource.hpp"

resource_manager_list & resource_manager_list::ref()
{
	static resource_manager_list man;
	return man;
}

void resource_manager_list::free()
{
	for (auto m : _managers)
		delete m;
}

void resource_manager_list::append(resource_manager * rman)
{
	_managers.push_back(rman);
}

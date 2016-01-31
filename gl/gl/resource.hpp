#pragma once
#include <vector>

class resource_manager
{
public:
	virtual ~resource_manager() {}
};

class resource_manager_list
{
public:
	static resource_manager_list & ref();
	void free();  //!< uvolni manazerov volanim delete
	void append(resource_manager * man);

private:
	std::vector<resource_manager *> _managers;
};

#pragma once
#include "luatools/luatools.hpp"
#include "resource_loader.hpp"

class lua_resource_loader : public resource_loader
{
public:
	lua_resource_loader();
	ptr<resource> create(std::string const & name, resource_manager * resman) override;

private:
	lua::vm _vm;
};

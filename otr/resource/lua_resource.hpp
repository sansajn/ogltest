#pragma once
#include <string>
#include "luatools/luatools.hpp"
#include "resource.hpp"

struct lua_resource_descriptor : public resource_descriptor
{
	lua_State * L;
	lua_resource_descriptor(std::string const & type, lua_State * L) : resource_descriptor(type), L(L) {}
};

#pragma once
#include <string>
#include <cassert>
#include "luatools/luatools.hpp"
#include "resource/resource.hpp"

struct lua_resource_descriptor : public resource_descriptor
{
	lua_State * L;
	lua_resource_descriptor(lua_State * L, std::string const & type) : resource_descriptor(type), L(L) {}
};

//! Lua helpers.
//@{
//! resource template helper
template <typename T>
class lua_resource : public resource_template<T>
{
protected:
	lua_State * state(resource_descriptor * desc);
};

template <typename T>
lua_State * lua_resource<T>::state(resource_descriptor * desc)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	assert(ldesc && "bad-cast");
	return ldesc->L;
}

//! Vrati nemo globalnej premennej na ktoru sa odkazuje tabulka v poly \c fieldname.
std::string get_variable_name(lua_State * L, std::string const & fieldname);

//@}

#include "lua_resource.hpp"
#include <map>
#include <cassert>
#include "luatools/table.hpp"

//! Vrati nazov premennej na ktoru sa odkazuje premenna na vrchole zasobnika (tabulka).
std::string refers_to_gloabal_variable(lua_State * L)  // TODO: kniznica
{
	assert(lua_istable(L, -1) && "table expected");

	lua_getglobal(L, "_G");  // maps global variables
	std::map<ptrdiff_t, std::string> variables;
	for (lua::table_range r(L); r; ++r)
	{
		if (r->key_type() == LUA_TSTRING)
			variables[ptrdiff_t(lua_topointer(L, -1))] = r->key();
	}
	lua_pop(L, 1);

	auto it = variables.find(ptrdiff_t(lua_topointer(L, -1)));
	assert(it != variables.end() && "no such global variable");
	return it->second;
}

std::string get_variable_name(lua_State * L, std::string const & fieldname)
{
	lua::table_field(L, fieldname);
	assert(lua_istable(L, -1) && "table expected");

	std::string name = refers_to_gloabal_variable(L);
	lua_pop(L, 1);  // pops value

	return name;
}

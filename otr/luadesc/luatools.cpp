#include "luatools.hpp"

namespace loe {
	namespace lua {

int traceback(lua_State * L);

void vm::init(lua_State * L)
{
	lua_gc(L, LUA_GCSTOP, 0);
	luaL_openlibs(L);
	lua_gc(L, LUA_GCRESTART, 0);
}

int vm::run_script(lua_State * L, char const * fname)
{
	int state = luaL_loadfile(L, fname);
	if (state == 0)
		state = call_chunk(L, 0);	
	return report(L, state);
}

int vm::call_chunk(lua_State * L, int narg)
{
	int trace_idx = lua_gettop(L) - narg;
	lua_pushcfunction(L, traceback);
	lua_insert(L, trace_idx);
	int state = lua_pcall(L, narg, LUA_MULTRET, trace_idx);
	lua_remove(L, trace_idx);
	if (state != 0)
		lua_gc(L, LUA_GCCOLLECT, 0);
	return state;
}

void vm::register_function(lua_State * L, lua_CFunction f, 
	char const * lname)
{
	lua_pushcfunction(L, f);
	lua_setglobal(L, lname);
}

int vm::call_function(lua_State * L, char const * lname, int narg)
{
	lua_getglobal(L, lname);	
	lua_insert(L, -(narg+1));
	int state = call_chunk(L, narg);
	return report(L, state);
}

int vm::report(lua_State * L, int state)
{
	if (state && !lua_isnil(L, -1))
	{
		char const * msg = lua_tostring(L, -1);
		if (!msg)
			msg = "(error object is not a string)";
		_luaerr(msg);
		lua_pop(L, 1);
	}
	return state;
}

int traceback(lua_State * L)
{
	if (!lua_isstring(L, 1))
		return 1;

	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}

	lua_pushvalue(L, 1);  // pass error message
	lua_pushinteger(L, 2);   // skip this function and traceback
	lua_call(L, 2, 1);  // call debug.traceback
	return 1;
}

	};  // lua
};  // loe



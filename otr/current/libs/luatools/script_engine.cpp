#include "script_engine.hpp"
#include "error_output.hpp"

namespace lua {

int traceback(lua_State * L);

result::result(lua_State * L, int nargs, int status)
	: _L(L), _nargs(nargs), _status(status)
{
	assert(nargs > -1 && "zaporna hodnota indexu zasobniku");
}

result::result(result && rhs)  // len pre istotu, inak RVO je pouzita
{
	_L = rhs._L;
	_nargs = rhs._nargs;
	_status = rhs._status;
	rhs._L = nullptr;
}

void result::flush()
{
	if (_L && _nargs)
		lua_pop(_L, _nargs);
	_nargs = 0;
}

script_engine & script_engine::default_vm()
{
	static script_engine lvm(lua::stderr_output);
	return lvm;
}

script_engine::script_engine(errout luaerr) : _L(luaL_newstate()), _luaerr(luaerr)
{
	if (!_L)
		throw lua_exception("unable to create lua state");

	lua_gc(_L, LUA_GCSTOP, 0);
	luaL_openlibs(_L);
	lua_gc(_L, LUA_GCRESTART, 0);
}

int script_engine::load_script(char const * fname)
{
	int state = luaL_loadfile(_L, fname);
	if (state == LUA_OK)
		state = call_chunk(_L, 0);
	assert(state != LUA_ERRFILE && "can't open script file");
	return report(_L, state);
}

int script_engine::call_chunk(lua_State * L, int narg)
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

void script_engine::register_function(lua_State * L, lua_CFunction f,
	char const * lname)
{
	lua_pushcfunction(L, f);
	lua_setglobal(L, lname);
}

void script_engine::global_variable(char const * name)
{
	lua_getglobal(_L, name);
}

int script_engine::call_function_raw(char const * lname, int narg)
{
	lua_getglobal(_L, lname);
	lua_insert(_L, -(narg+1));
	int state = call_chunk(_L, narg);
	return report(_L, state);
}

result script_engine::call_function_impl(char const * lname, int nargs)
{
	int n = lua_gettop(_L) - nargs;
	lua_getglobal(_L, lname);
	lua_insert(_L, -(nargs+1));
	int state = call_chunk(_L, nargs);
	report_if_error(state);
	return result(_L, lua_gettop(_L) - n, state);
}

void script_engine::report_if_error(int state)
{
	report(_L, state);
}

int script_engine::report(lua_State * L, int state)
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

}  // lua

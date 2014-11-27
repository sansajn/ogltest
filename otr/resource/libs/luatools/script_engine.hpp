#pragma once
#include <stdexcept>
#include <cassert>
#include <lua.hpp>
#include "stack.hpp"

namespace lua {

class lua_exception : std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

typedef void (* errout)(char const * msg);

//! Automaticka sprava zasobniku po navrate z volania lua funkcie.
class result
{
public:
	result(result && rhs);
	~result() {flush();}

	int status() const {return _status;}  //!< navratova hodnota call_chunk()
	lua_State * state() const {return _L;}
	void flush();

private:
	result(lua_State * L, int nargs, int status);

	lua_State * _L;
	int _nargs;  //!< number of returned arguments (by function call)
	int _status;

	friend class script_engine;
};  // result

//! Umoznuje nacitat skripty, spustat funkcie, ziskavat premenne ...
class script_engine
{
public:
	static script_engine & default_vm();

	script_engine(errout luaerr = nullptr);
	~script_engine() {lua_close(_L);}

	int load_script(char const * fname);
	void register_function(lua_State * L, lua_CFunction, char const * lname);

	result call_function(char const * lname) {return call_function_impl(lname, 0);}

	template <typename T, typename... Args>
	result call_function(char const * lname, T head, Args ... args)
	{
		return call_function_impl(lname, 0, head, args ...);
	}

	int call_function_raw(char const * lname, int narg);  // zavola funkciu bez resultu

	template <typename R>
	R global_variable(char const * name)  //!< Vrati globalnu premennu 'name'.
	{
		assert(!lua_istable(_L, -1) && "not working with tables, use global_variable_raw() instead");
		global_variable(name);
		if (lua_isnil(_L, -1))
			throw lua_exception("unknown variable");
		return stack_pop<R>(_L);
	}

	void global_variable(char const * name);  //!< na vrchol zasobnika ulozi globalnu premennu 'name'

	template <typename T>
	void global_variable(char const * name, T const & val)  //!< vytvori globalnu premennu 'name' s hodnotou val
	{
		stack_push(_L, val);
		lua_setglobal(_L, name);
	}

	lua_State * state() const {return _L;}

private:
	int report(lua_State * L, int state);  // TODO: odstran
	void report_if_error(int state);
	int call_chunk(lua_State * L, int narg);

	template <typename T, typename... Args>
	result call_function_impl(char const * lname, int nargs, T head, Args ... args)
	{
		stack_push(_L, head);
		return call_function_impl(lname, nargs+1, args ...);
	}

	result call_function_impl(char const * lname, int nargs);  // stop function

	lua_State * _L;
	errout _luaerr;
};  // script_engine

}  // lua

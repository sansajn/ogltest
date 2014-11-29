#pragma once
#include <string>
#include <utility>
#include <lua.hpp>
#include "stack.hpp"

namespace lua {

// TODO: odstran retazec ako kluc
//! Iteracia skrz homogennu tabulku.
template <typename T>
class map_range
{
public:
	map_range(lua_State * L, int sidx = -1) : _L(L) {
		_sidx = lua_gettop(_L) + (sidx + 1);
		lua_pushnil(_L);
		_ok = lua_next(_L, _sidx);
	}

	void operator++() {
		lua_pop(_L, 1);
		_ok = lua_next(_L, _sidx);
	}

	std::pair<std::string, T> operator*() {
		return std::make_pair(stack_at<std::string>(_L, -2), stack_at<T>(_L, -1));
	}

	operator bool() const {return _ok;}

private:
	bool _ok;
	int _sidx;  //!< table stack index
	lua_State * _L;
};  // map_range

}  // lua

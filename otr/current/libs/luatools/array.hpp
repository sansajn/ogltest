#pragma once
#include <lua.hpp>
#include "stack.hpp"

namespace lua {

/*! Umoznuje iterovat polia */
template <typename T>
class array_range
{
public:
	array_range(lua_State * L, int sidx) : _L(L), _sidx(sidx), _tidx(1) {}
	void operator++() {++_tidx;}

	T operator*() {
		lua_rawgeti(_L, _sidx, _tidx);
		return stack_pop<T>(_L);
	}

	operator bool() {return _tidx <= lua_rawlen(_L, _sidx);}

private:
	lua_State * _L;
	int _tidx;  //!< table index
	int const _sidx;  //!< stack index
};  // array_range

}  // lua

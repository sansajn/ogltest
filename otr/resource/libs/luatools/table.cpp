#include "table.hpp"

namespace lua {

table_iterator::table_iterator(lua_State * L)
	: _L(L)
{
	if (!_L)
		return;
	lua_pushnil(_L);
	next();
}

table_iterator & table_iterator::operator++()
{
	lua_pop(_L, 1);
	next();
	return *this;
}

void table_iterator::next()
{
	assert(lua_istable(_L, -2) && "table expected");
	int res = lua_next(_L, -2);
	if (!res)
		_L = nullptr;
}

table::table(table && rhs)
{
	_L = rhs._L;
	_sidx = rhs._sidx;
	_destroy = rhs._destroy;
	rhs._L = nullptr;
}

table::~table()
{
	if (_L && _destroy)
		pop();
}

void table::newtable()
{
	lua_newtable(_L);
	_sidx = lua_gettop(_L);
	_destroy = true;
}

void table::pop()
{
	assert(lua_gettop(_L) == _sidx && "tabulka musi byt na vrchole zasobnika");
	lua_pop(_L, 1);
	_destroy = false;
}

}  // lua

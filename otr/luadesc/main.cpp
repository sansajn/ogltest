// pokus o rozparsovanie resource.lua
#include <iostream>
#include "luatools.hpp"
#include "error_message.hpp"

using namespace std;

void print_table(lua_State * L);
void print_attribute(lua::table_range & r);


int main(int argc, char * argv[])
{
	lua::vm lvm(lua::lmessage);
	lvm.load_script("resource.lua");

	lua_State * L = lvm.state();
	lua_getglobal(L, "content");  // na zasobniku je content
	assert(lua_istable(L, -1) && "table expected");

/*
	lua::result res = lvm.call_function("get_content");

	// keys in content
	for (lua::table_range r(res); r; ++r)
	{
		switch (r.key_type())
		{
			case LUA_TSTRING:
			{
				print_attribute(r);
				break;
			}

			case LUA_TNUMBER:
				cout << r.key<int>() << ":table\n";
				break;

			default:
				assert(false);
		}
	}
*/

	print_table(L);


	return 0;
}

void print_attribute(lua::table_range & r)
{
	switch (r.value_type())
	{
		case LUA_TNIL:
			cout << r.key() << ":nil\n";
			break;

		case LUA_TBOOLEAN:
			cout << r.key() << ":" << r.value<bool>() << "\n";
			break;

		case LUA_TNUMBER:
			cout << r.key() << ":" << r.value<lua_Number>() << "\n";
			break;

		case LUA_TSTRING:
			cout << r.key() << ":" << r.value<string>() << "\n";
			break;

		case LUA_TTABLE:
			assert(false && "table not allowed");
			break;

		case LUA_TTHREAD:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
			cout << r.key() << ":...\n";
			break;
	}
}

void print_table(lua_State * L)
{
	for (lua::table_range r(L); r; ++r)
	{
		if (r.key_type() == LUA_TNUMBER)
			print_table(L);
		else if (r.key_type() == LUA_TSTRING)
			print_attribute(r);
		else
			assert(false && "only string and number keys allowed");
	}
}

#define PRINT_TOP()(cout << "top:" << lua_gettop(L) << "\n")


#include <iostream>
#include "luatools.hpp"
#include "error_message.hpp"

using namespace std;

#define CHECK_TABLE() (assert(lua_istable(L, -1) && "table expected"))

int main(int argc, char * argv[])
{
	lua::vm lvm(lua::lmessage);
	lvm.load_script("adr.lua");
	lua_State * L = lvm.state();
	lua_getglobal(L, "b");
	CHECK_TABLE();

	int table_idx = lua_gettop(L);

	// Ako ziskam adresu tabulky (tu adresu, ktoru vrati vyraz tostring(t) -> 'table: 0x9384cc8') ?
	void const * ptr = lua_topointer(L, -1);
	cout << "b:" << ptr << "\n";

	lua_pushnil(L);
	lua_next(L, table_idx);  // value:-1, key:-2
	CHECK_TABLE();

	void const * ptr2 = lua_topointer(L, -1);
	cout << "b[1]:" << ptr2 << "\n";

	lua_pop(L, 1);
	lua_next(L, table_idx);
	CHECK_TABLE();
	void const * ptr3 = lua_topointer(L, -1);
	cout << "b[2]:" << ptr3 << "\n";

	return 0;
}

#include "luatools.hpp"

int lua_foreach(lua_State * L);

int main(int argc, char * argv[])
{
	return 0;
}


struct foreach_obj
{
	string var;
	set<string> flags;
	bool parralel;
	bool culling;
};

/* foreach : var, flag, parallel, culling */
int lua_foreach(lua_State * L)
{
	lua::istack_stream is(L);
	foreach_obj o;


//	o <<
	return 0;
}

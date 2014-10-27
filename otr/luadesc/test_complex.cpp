// complex user-defined structure example
#include <string>
#include "luatools.hpp"
using std::string;
namespace lua = loe::lua;

struct longposition
{
	long x, y;
};

struct trip_desc
{
	string from_name;
	longposition from_pos;
	string to_name;
	longposition to_pos;
};


void lmessage(char const * msg);


namespace loe {
	namespace lua {

//! tieto funkcie neprebijú členský operator<<

inline ostack_stream & operator<<(ostack_stream & os, longposition const & pos)
{
/*
	ostack_stream(os.get()) 
		<< lua::newtable << lua::tab("lat", pos.y) << lua::tab("lon", pos.x);
*/
	return os;
}

inline istack_stream & operator>>(istack_stream & is, longposition & p)
{
//	is >> lua::tab( "lat", p.y ) >> lua::tab( "lon", p.x) >> lua::next;
	return is;
}

/*
inline ostack_stream & operator<<(ostack_stream & os, 
	trip_desc const & trip)
{
	ostack_stream(os.get())	
		<< lua::newtable 
		<< lua::tab("from_name", trip.from_name)
		<< lua::tab("from_pos", trip.from_pos)
		<< lua::tab("to_name", trip.to_name)
		<< lua::tab("to_pos", trip.to_pos);
	return os;

	// ? prečo musím vytvárať novy stream ?
	// os << lua::newtable << ...;
}
*/

	}  // lua
}  // loe


int main(int argc, char * argv[])
{
	trip_desc trip;
	trip.from_name = "a";
	trip.to_name = "b";
	trip.from_pos = longposition{1, 2};
	trip.to_pos = longposition{3, 5};

	lua::vm lvm(lmessage);	
	lua_State * L = lua::newstate();
	lvm.init(L);
	lvm.run_script(L, "complex_structure.lua");

	lua::ostack_stream oss(L);
	oss << trip;

	lvm.call_function(L, "echo", 1);

	lua_close(L);
	return 0;
}

void lmessage(char const * msg)
{
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
}


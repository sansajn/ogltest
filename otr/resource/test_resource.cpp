// pokus o odstrnenie nutnosti ovodzovt zdroje od resource
#include <map>
#include <set>
#include <vector>
#include <string>
#include "luatools/luatools.hpp"
#include "luatools/error_output.hpp"
#include "core/ptr.hpp"
#include "resource.hpp"
#include "resource_loader.hpp"
#include "resource_factory.hpp"
#include "resource_manager.hpp"
#include "lua_resource.hpp"
#include "lua_resource_loader.hpp"
#include "lua_scenegraph.hpp"

using namespace std;


string refers_to_gloabal_variable(lua_State * L);  // TODO: zaclen do kniznice
void table_field(lua_State * L, char const * name);  // TODO: zaclen do kniznice

template <typename R>
R table_field(lua_State * L, const char * name);  // TODO: zaclen do kniznice

string read_value_attribute(lua_State * L);

resource_factory::type<lua_node_resource> node_type("node");
resource_factory::type<lua_foreach_factory_resource> foreach_factory_type("foreach");
resource_factory::type<lua_callmethod_factory_resource> callmethod_factory_type("callmethod");


int main(int argc, char * argv[])
{
	auto factory = resource_factory::ref();
	resource_manager resman(make_ptr<lua_resource_loader>());
	ptr<node> scene = resman.load_resource<node>("scene");
	return 0;
}




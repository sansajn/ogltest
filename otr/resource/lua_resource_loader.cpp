#include "lua_resource_loader.hpp"
#include "luatools/error_output.hpp"
#include "lua_resource.hpp"
#include "resource_factory.hpp"

lua_resource_loader::lua_resource_loader() : _vm(lua::stderr_output)
{
	_vm.load_script("helloworld.lua");
}

ptr<resource> lua_resource_loader::create(std::string const & name, resource_manager * resman)
{
	lua_State * L = _vm.state();
	lua_getglobal(L, name.c_str());  // precitaj premennu 'name'
	assert(!lua_isnil(L, -1) && "variable 'name' doesn't exists");

	lua::table tb(L);
	std::string type = tb.at("type");
	lua_resource_descriptor desc(type, L);
	ptr<resource> r = resource_factory::ref().create(&desc, resman);

	lua_pop(L, 1);  // pops name variable
	return r;
}

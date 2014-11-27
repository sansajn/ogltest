#include "lua_resource_loader.hpp"
#include <fstream>
#include <boost/filesystem.hpp>
#include "luatools/table.hpp"
#include "luatools/error_output.hpp"
#include "core/utils.hpp"
#include "lua_resource.hpp"
#include "resource_factory.hpp"

namespace fs = boost::filesystem;

lua_resource_loader::lua_resource_loader(const char * fname) : _vm(lua::stderr_output)
{
	_vm.load_script(fname);
}

ptr<resource> lua_resource_loader::create(std::string const & name, resource_manager & resman)
{
	// name sa moze odkazovat na premennu, subor, alebo je to popis, ako vyrobit resource 'na mieste'
	if (name.rfind(';') != std::string::npos)
		return create_from_description("program", name, resman);
	else if (name.rfind('.') != std::string::npos)
		return create_from_file(name, resman);
	else
		return create_from_variable(name, resman);
}


ptr<resource> lua_resource_loader::create_from_variable(std::string const & name, resource_manager & resman)
{
	lua_State * L = _vm.state();
	_vm.global_variable(name.c_str());
	assert(!lua_isnil(L, -1) && "unknown 'name' variable");

	lua::table tb(L);
	std::string type = tb.at("type");
	lua_resource_descriptor desc(L, type);
	ptr<resource> r = resource_factory::ref().create(desc, resman);

	lua_pop(L, 1);  // pops 'name' variable
	return r;
}

ptr<resource> lua_resource_loader::create_from_description(std::string const & type, std::string const & name, resource_manager & resman)
{
	if (type == "program")  // TODO: namiesto retazca enum
	{
		lua_State * L = _vm.state();
		std::string type = "program";
		lua::table tb(L);
		tb.newtable();
		tb.at("type") = type;
		tb.at("modules") = name;
		lua_resource_descriptor desc(L, type);
		return resource_factory::ref().create(desc, resman);
	}

	assert(false && "unknown resource type");
	throw std::exception();  // TODO: specify
}

ptr<resource> lua_resource_loader::create_from_file(std::string const & name, resource_manager & resman)
{
	lua_State * L = _vm.state();

	lua::table tb(L);
	std::string type = tb.at("type");
	lua_resource_descriptor desc(L, type);

	return resource_factory::ref().create(desc, resman);
}

std::string lua_resource_loader::find_resource(std::string const & name) const
{
	if (fs::exists(fs::path(name)))  // .
		return name;

	for (auto d : _paths)
	{
		fs::path p(fs::path(d)/name);
		if (fs::exists(p))
			return p.native();
	}

	assert(false && "resource not found");

	return name;
}

std::string lua_resource_loader::load_text(std::string const & fname) const
{
	return read_file(find_resource(fname));
}

ptr<uint8_t> lua_resource_loader::load_binary(std::string const & fname, size_t & size) const
{
	std::ifstream fin(find_resource(fname));
	if (!fin.is_open())
		return nullptr;

	fin.seekg(0, std::ios::end);
	size = fin.tellg();
	fin.seekg(0);
	ptr<uint8_t> buf = make_ptr<uint8_t>(size);
	fin.read((char *)buf.get(), size);
	assert(size == fin.gcount() && "not all bytes were reed");
	return buf;
}

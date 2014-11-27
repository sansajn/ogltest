#pragma once
#include "luatools/luatools.hpp"
#include "resource_loader.hpp"

class lua_resource_loader : public resource_loader
{
public:
	lua_resource_loader(char const * fname);
	ptr<resource> create(std::string const & name, resource_manager & resman) override;
	std::string find_resource(std::string const & name) const;
	void append_path(std::string const & path) {_paths.push_back(path);}
	std::string load_text(std::string const & fname) const;
	ptr<uint8_t> load_binary(std::string const & fname, size_t & size) const;

private:
	ptr<resource> create_from_description(std::string const & type, std::string const & name, resource_manager & resman);
	ptr<resource> create_from_variable(std::string const & name, resource_manager & resman);
	ptr<resource> create_from_file(std::string const & name, resource_manager & resman);

	lua::script_engine _vm;
	std::vector<std::string> _paths;
};

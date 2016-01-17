#include "resource.hpp"

using std::string;

path_manager & path_manager::ref()
{
	static path_manager rman;
	return rman;
}

void path_manager::root_path(std::string const & p)
{
	_root_path = p;
}

std::string path_manager::translate_path(std::string const & p)
{
	return _root_path + "/" + p;
}

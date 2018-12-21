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

string path_manager::translate_path(char const * p) const
{
	return translate_path(string{p});
}

string path_manager::translate_path(string const & p) const
{
	return _root_path + "/" + p;
}

#pragma once
#include <string>

class path_manager
{
public:
	static path_manager & ref();
	void root_path(std::string const & p);
	std::string translate_path(std::string const & p);

private:
	path_manager() {}

	std::string _root_path;
};

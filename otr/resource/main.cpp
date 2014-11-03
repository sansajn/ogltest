#include <string>
#include <iostream>
#include "resource/resource_manager.hpp"


int main(int argc, char * argv[])
{
	resource_manager man;
	ptr<std::string> r(new std::string("Teresa Lisbon"));
	man.insert_resource("user", r);
	auto s = man.load_resource<std::string>("user");
	if (s)
		std::cout << "user:" << *s << std::endl;
	else
		std::cout << "user is not an instance of requested-type" << std::endl;
	return 0;
}

#include <string>
#include <iostream>
#include "resource/resource_manager.hpp"


class string_res : public resource
{
public:
	string_res(std::string const & name, std::string const & content)
		: resource(name), _content(content)
	{}

	std::string const & data() const {return _content;}

private:
	std::string _content;
};


int main(int argc, char * argv[])
{
	resource_manager man;
	ptr<string_res> r(new string_res("user", "Teresa Lisbon"));
	man.insert_resource(r->name(), r);
	auto s = man.load_resource<string_res>("user");
	std::cout << s->name() << ":" << s->data() << std::endl;
	return 0;
}

#include <string>
#include <iostream>
#include "resource/resource_manager.hpp"

using namespace std;

struct foo
{
	int data;
	foo(int d) : data(d) {}
};

class foo_resource : public resource, public foo
{
public:
	foo_resource(int data) : resource(), foo(data) {}
};


int main(int argc, char * argv[])
{
	resource_manager man;
	man.insert_resource("dummy", make_ptr<foo_resource>(101));
	ptr<foo> f = man.load_resource<foo>("dummy");
	assert(f->data == 101 && "to same as expected");
	cout << f->data << "\n";
	return 0;
}

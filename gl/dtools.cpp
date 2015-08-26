#include "dtools.hpp"
#include <iostream>

using std::cout;
using std::endl;

void print_vec3(glm::vec3 const v, std::string const & name)
{
	cout << name << ": " << v.x << " " << v.y << " " << v.z << endl;
}

void print_quat(glm::quat const & q, std::string const & name)
{
	cout << name << ": " << q.x << " " << q.y << " " << q.z << " " << q.w << endl;
}

#include <iostream>
#include <glm/glm.hpp>
#include "render/any_value.hpp"


int main(int argc, char * argv[])
{
	any_value1f v1("dummy", 12.34f);
	any_value2f v2("dummy", glm::vec2(1.0f, 9.0f));
	any_value_matrix3f v3("dummy", glm::mat3(1.0f));
	if (glm::mat3(1.0f) == v3.get_matrix())
		std::cout << "equal\n";
	return 0;
}

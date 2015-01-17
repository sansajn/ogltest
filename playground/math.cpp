#include "math.hpp"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 transform::transformation() const
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 R = glm::mat4_cast(rotation);
	glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
	return T*R*S;
}

void transform::rotate(glm::vec3 axis, float angle)
{
	rotate(glm::angleAxis(angle, axis));
}

void transform::rotate(glm::quat rot)
{
	rotation = glm::normalize(rot * rotation);
}

void transform::uniform_scale(float s)
{
	scale = glm::vec3(s, s, s);
}

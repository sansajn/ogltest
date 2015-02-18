#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

camera::camera(float fov, float aspect, float near, float far)
	: _P(glm::perspective(fov, aspect, near, far))
{}

camera::camera(glm::vec3 const & pos, float fov, float aspect, float near, float far)
	: camera(fov, aspect, near, far)
{
	position = pos;
}

glm::mat4 camera::view() const
{
	glm::mat4 R = glm::mat4_cast(glm::conjugate(rotation));
	glm::mat4 T = glm::translate(glm::mat4(1.0f), -position);
	return R * T;  // inverzna matica, preto R * T
}

glm::vec3 camera::right() const
{
	glm::mat3 R = glm::mat3_cast(rotation);
	return R[0];
}

glm::vec3 camera::up() const
{
	glm::mat3 R = glm::mat3_cast(rotation);
	return R[1];
}

glm::vec3 camera::forward() const
{
	glm::mat3 R = glm::mat3_cast(rotation);
	return R[2];
}

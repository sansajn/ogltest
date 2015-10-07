#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "math.hpp"

camera::camera(glm::mat4 const & proj)	: _P(proj)
{}

camera::camera(float fov, float aspect, float near, float far)
	: _P(glm::perspective(fov, aspect, near, far))
{}

glm::mat4 camera::view_projection() const
{
	glm::mat4 R = glm::mat4_cast(glm::conjugate(transformation.rotation));
	glm::mat4 T = glm::translate(glm::mat4(1.0f), -transformation.position);
	return _P * R * T;  // je to inverzna transformacia, preto R*T (najprv posunie, potom odrotuje)
}

glm::vec3 camera::right() const
{
	glm::mat3 R = glm::mat3_cast(transformation.rotation);
	return R[0];
}

glm::vec3 camera::up() const
{
	glm::mat3 R = glm::mat3_cast(transformation.rotation);
	return R[1];
}

glm::vec3 camera::forward() const
{
	glm::mat3 R = glm::mat3_cast(transformation.rotation);
	return R[2];
}

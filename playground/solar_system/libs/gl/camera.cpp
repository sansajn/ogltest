#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace gl {

camera::camera(float fovy, float aspect, float near, float far)
	: _proj{glm::perspective(fovy, aspect, near, far)}
{}

camera::camera(glm::vec3 const & pos, float fovy, float aspect, float near, float far)
	: camera(fovy, aspect, near, far)
{
	position = pos;
}

void camera::look_at(glm::vec3 const & center)
{
	glm::vec3 to_camera_dir = position - center;

	assert(glm::length(to_camera_dir) > 0.001f && "center is too close to position");

	glm::vec3 f = glm::normalize(to_camera_dir);  // forward
	glm::vec3 u = glm::vec3(0,1,0);  // up
	glm::vec3 r = glm::normalize(glm::cross(u,f));  // right
	u = glm::cross(f,r);

	glm::mat4 M(
		r.x, r.y, r.z, 0,
		u.x, u.y, u.z, 0,
		f.x, f.y, f.z, 0,
		  0,   0,   0, 1);

	rotation = glm::quat_cast(M);
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

void camera::projection(float fovy, float aspect, float near, float far)
{
	_proj = glm::perspective(fovy, aspect, near, far);
}

}  // gl

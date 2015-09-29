#include "camera.h"
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace gl {

camera::camera(glm::vec3 const & eye, glm::vec3 const & center)
{
	reset(eye, center);
	_speed = .5f;
}

void camera::reset(glm::vec3 const & eye, glm::vec3 const & center)
{
	glm::vec2 rot = compute_rotations(eye, center);
	_yaw = rot.x;
	_pitch = rot.y;
	_eye = eye;
	_V = compute_view(_yaw, _pitch, eye);
}

glm::vec2 camera::compute_rotations(glm::vec3 const & eye, glm::vec3 const & center)
{
	glm::vec3 diff = eye - center;
	float yaw = atan2(diff.x, diff.z);
	float pitch = -atan2(diff.y, sqrt(diff.x*diff.x + diff.z*diff.z));
	return glm::vec2(yaw, pitch);
}

glm::mat4 camera::compute_view(float yaw, float pitch, glm::vec3 const & eye)
{
	glm::mat4 R = glm::yawPitchRoll(yaw, pitch, 0.0f);
	glm::vec3 look = glm::vec3(R*glm::vec4(0, 0, -1, 0));
	glm::vec3 center = eye + look;
	glm::vec3 up = glm::vec3(R*glm::vec4(0, 1, 0, 0));
	return glm::lookAt(eye, center, up);
}

}  // gl

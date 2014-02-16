#include "free_camera.h"

#include <glm/gtx/transform.hpp>

namespace gl {

free_camera::free_camera() 
{
	_translation = glm::vec3(0);
	_speed = 0.5f;
}

void free_camera::update() 
{
	_position += _translation;

	glm::mat4 R = glm::yawPitchRoll(_yaw, _pitch, _roll);
	_look = glm::vec3(R*glm::vec4(0, 0, 1, 0));
	_up = glm::vec3(R*glm::vec4(0, 1, 0, 0));
	_right = glm::cross(look, up);

	glm::vec3 center = _position + _look;
	_V = glm::lookAt(_position, center, _up);
}

void free_camera::walk(float dt)
{
	_translation += _look*_speed*dt;
	update();
}

void free_camera::strafe(float dt)
{
	_translation += _right*_speed*dt;
	update();
}

void free_camera::lift(float dt)
{
	_translation += _up*_speed*dt;
	update();
}

void free_camera::translation(glm::vec3 const & t)
{
	_translation = t;
	update();
}

}  // gl

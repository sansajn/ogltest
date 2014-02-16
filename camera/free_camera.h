#pragma once

#include <glm/glm.hpp>

namespace gl {

class free_camera
{
public:
	free_camera();
	void rotate(float yaw, float pitch, float roll);
	void walk(float dt);
	void strafe(float dt);
	void lift(float dt);

	void translation(glm::vec3 const & t);
	glm::vec3 translation() const {return _translation;}

	void speed(float s) {_speed = s;}
	float speed() const {return _speed;}

	glm::mat4 const & view_matrix() const {return _V;}

	void position(glm::vec3 const & p);
	glm::vec3 const & position() const {return _position;}

private:
	void update();

	glm::vec3 _look;
	glm::vec3 _right;
	glm::vec3 _up;
	glm::vec3 _position;
	float _yaw, _pitch, _roll;
	float _speed;
	glm::vec3 _translation;
	glm::mat4 _V;
};


}  // gl

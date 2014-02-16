#pragma once
#include <glm/glm.hpp>

namespace gl {

class camera
{
public:
	camera() : _speed(.5f), _V(1.0f) {}
	camera(glm::vec3 const & eye, glm::vec3 const & center);
	void reset(glm::vec3 const & eye, glm::vec3 const & center);
	void update() {_V = compute_view(_yaw, _pitch, _eye);}

	void up(float dt)	{_pitch += _speed*dt;}
	void down(float dt) {_pitch -= _speed*dt;}
	void left(float dt) {_yaw += _speed*dt;}
	void right(float dt) {_yaw -= _speed*dt;}

	glm::mat4 const & view() const {return _V;}

	void speed(float s) {_speed = s;}
	float speed() const {return _speed;}

	glm::vec3 rotation() const {return glm::vec3(_yaw, _pitch, 0);}

private:
	glm::mat4 compute_view(float yaw, float pitch, glm::vec3 const & eye);
	glm::vec2 compute_rotations(glm::vec3 const & eye, glm::vec3 const & center);

	float _yaw;
	float _pitch;
	float _speed;  // uhlova rychlost otacania
	glm::vec3 _eye;  // pozicie kamery
	glm::mat4 _V;  // view-matrix
};

}  // gl

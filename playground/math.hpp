#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

class transform
{
public:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	transform() : position(0, 0, 0), rotation(1, 0, 0, 0), scale(1, 1, 1) {}

	glm::mat4 transformation() const;
	void rotate(glm::vec3 axis, float angle);
	void rotate(glm::quat rot);
	void uniform_scale(float s);
};

#pragma once
#include <glm/mat4x4.hpp>
#include "math.hpp"

class camera
{
public:
	transform transformation;

	camera(glm::mat4 const & proj);
	camera(float fov, float aspect, float near, float far);

	glm::mat4 projection() const {return _P;}
	glm::mat4 view_projection() const;

	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;

private:
	glm::mat4 _P;
};

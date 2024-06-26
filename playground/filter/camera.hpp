#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

/*! Camera class implementation.
\code
	camera c(70, float(w)/float(h), 0.01, 1000);
\endcode */
class camera
{
public:
	glm::vec3 position;
	glm::quat rotation;

	camera(float fov, float aspect, float near, float far);
	camera(glm::vec3 const & pos, float fov, float aspect, float near, float far);

	glm::mat4 view() const;
	glm::mat4 const & projection() const {return _P;}
	glm::mat4 view_projection() const {return _P * view();}

	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;

private:
	glm::mat4 _P;
};

#pragma once
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>

inline std::ostream & operator<<(std::ostream & o, glm::quat const & q)
{
	o << "[(" << q.x << ", " << q.y << ", " << q.z << "), " << q.w << "]";
	return o;
}

inline std::ostream & operator<<(std::ostream & o, glm::vec3 const & v)
{
	o << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return o;
}

inline std::ostream & operator<<(std::ostream & o, glm::vec4 const & v)
{
	o << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	return o;
}

inline std::ostream & operator<<(std::ostream & o, glm::mat3 const & m)
{
	o << "[" << m[0] << ", " << m[1] << ", " << m[2] << "]";
	return o;
}

inline std::ostream & operator<<(std::ostream & o, glm::mat4 const & m)
{
	o << "[" << m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << "]";
	return o;
}

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class plane  //!< geometric plane implementation
{
public:
	enum class halfspace
	{
		negative, on_plane, positive
	};

	float a, b, c, d;

	plane() {}
	plane(float a, float b, float c, float d) : a(a), b(b), c(c), d(d) {}
	void normalize();
	float distance_to(glm::vec3 const & p) const;  //!< \note vrati skutocnu vzdialenost len ak je rovina normlizovana
	halfspace classify(glm::vec3 const & p) const;
};

class frustum
{
public:
	frustum(glm::mat4 const & combined_matrix);  //!< \param combined_matrix another word for MVP matrix

	bool point_in_frustum(glm::vec3 const & p) const;
	bool sphere_in_frustum(glm::vec3 const & center, float radius) const;
	bool box_in_frustum(glm::vec3 const & nim_corner, glm::vec3 const & max_corner) const;

private:
	void extract_planes(glm::mat4 const & m, bool norm);

	plane _planes[6];
};

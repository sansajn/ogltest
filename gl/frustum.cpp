#include "frustum.hpp"
#include <cmath>

using std::sqrt;


frustum::frustum(glm::mat4 const & combined_matrix)
{
	extract_planes(combined_matrix, true);
}

bool frustum::point_in_frustum(glm::vec3 const & p) const
{
	for (int i = 0; i < 6; ++i)
		if (_planes[i].classify(p) == plane::halfspace::negative)
			return false;
	return true;
}

bool frustum::sphere_in_frustum(glm::vec3 const & center, float radius) const
{
	for (int i = 0; i < 6; ++i)
		if (_planes[i].distance_to(center) <= -radius)
			return false;
	return true;
}

/*! Pred kazdou rovinou musi byt aspon jeden vrchol, potom vrati true.
TODO: zlepsi detekciu */
bool frustum::box_in_frustum(glm::vec3 const & min_corner, glm::vec3 const & max_corner) const
{
	for (plane const & pl : _planes)
	{
		glm::vec3 pt = min_corner;
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{max_corner.x, min_corner.y, min_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{max_corner.x, min_corner.y, max_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{min_corner.x, min_corner.y, max_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = max_corner;
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{max_corner.x, max_corner.y, min_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{min_corner.x, max_corner.y, min_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		pt = glm::vec3{min_corner.x, max_corner.y, min_corner.z};
		if (pl.classify(pt) == plane::halfspace::positive)
			continue;

		return false;
	}

	return true;
}

void frustum::extract_planes(glm::mat4 const & m, bool norm)
{
	// left clipping plane
	_planes[0].a = m[0][3] + m[0][0];
	_planes[0].b = m[1][3] + m[1][0];
	_planes[0].c = m[2][3] + m[2][0];
	_planes[0].d = m[3][3] + m[3][0];

	// right clipping plane
	_planes[1].a = m[0][3] - m[0][0];
	_planes[1].b = m[1][3] - m[1][0];
	_planes[1].c = m[2][3] - m[2][0];
	_planes[1].d = m[3][3] - m[3][0];

	// top clipping plane
	_planes[2].a = m[0][3] - m[0][1];
	_planes[2].b = m[1][3] - m[1][1];
	_planes[2].c = m[2][3] - m[2][1];
	_planes[2].d = m[3][3] - m[3][1];

	// bottom clipping plane
	_planes[3].a = m[0][3] + m[0][1];
	_planes[3].b = m[1][3] + m[1][1];
	_planes[3].c = m[2][3] + m[2][1];
	_planes[3].d = m[3][3] + m[3][1];

	// near clipping plane
	_planes[4].a = m[0][3] + m[0][2];
	_planes[4].b = m[1][3] + m[1][2];
	_planes[4].c = m[2][3] + m[2][2];
	_planes[4].d = m[3][3] + m[3][2];

	// far clipping plane
	_planes[5].a = m[0][3] - m[0][2];
	_planes[5].b = m[1][3] - m[1][2];
	_planes[5].c = m[2][3] - m[2][2];
	_planes[5].d = m[3][3] - m[3][2];

	if (norm)  // normalize planes if requested
	{
		_planes[0].normalize();
		_planes[1].normalize();
		_planes[2].normalize();
		_planes[3].normalize();
		_planes[4].normalize();
		_planes[5].normalize();
	}
}

void plane::normalize()
{
	float mag = sqrt(a*a + b*b + c*c);
	a /= mag;
	b /= mag;
	c /= mag;
	d /= mag;
}

float plane::distance_to(glm::vec3 const & p) const
{
	return a*p.x + b*p.y + c*p.z + d;
}

plane::halfspace plane::classify(glm::vec3 const & p) const
{
	float d = distance_to(p);
	if (d < 0.0f)
		return halfspace::negative;
	if (d > 0.0f)
		return halfspace::positive;
	else
		return halfspace::on_plane;
}

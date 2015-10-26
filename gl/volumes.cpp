#include "volumes.hpp"
#include <GL/glew.h>

using glm::vec3;
using gl::mesh;
using gl::attribute;

mesh make_cube()
{
	float verts[6*4*(3+3)] = {  // position:3, normal:3
		// front
		-0.5, -0.5, 0.5,  0, 0, 1,
		0.5, -0.5, 0.5,  0, 0, 1,
		0.5, 0.5, 0.5,  0, 0, 1,
		-0.5, 0.5, 0.5,  0, 0, 1,
		// right
		0.5, -0.5, 0.5,  1, 0, 0,
		0.5, -0.5, -0.5,  1, 0, 0,
		0.5, 0.5, -0.5,  1, 0, 0,
		0.5, 0.5, 0.5,  1, 0, 0,
		// top
		-0.5, 0.5, 0.5,  0, 1, 0,
		0.5, 0.5, 0.5,  0, 1, 0,
		0.5, 0.5, -0.5,  0, 1, 0,
		-0.5, 0.5, -0.5,  0, 1, 0,
		// bottom
		-0.5, -0.5, -0.5,  0, -1, 0,
		0.5, -0.5, -0.5,  0, -1, 0,
		0.5, -0.5, 0.5,  0, -1, 0,
		-0.5, -0.5, 0.5,  0, -1, 0,
		// back
		0.5, -0.5, -0.5,  0, 0, -1,
		-0.5, -0.5, -0.5,  0, 0, -1,
		-0.5, 0.5, -0.5,  0, 0, -1,
		0.5, 0.5, -0.5,  0, 0, -1,
		// left
		-0.5, -0.5, -0.5,  -1, 0, 0,
		-0.5, -0.5, 0.5,  -1, 0, 0,
		-0.5, 0.5, 0.5,  -1, 0, 0,
		-0.5, 0.5, -0.5,  -1, 0, 0
	};

	unsigned indices[] = {
		0, 1, 2,  2, 3, 0,
		4, 5, 6,  6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	mesh m = mesh{verts, sizeof(verts), indices, 2*6*3};
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 3*sizeof(GL_FLOAT)});
	return m;
}

gl::mesh make_box(vec3 const & half_extents)
{
	vec3 const & h = half_extents;

	float verts[6*4*(3+3)] = {  // position:3, normal:3
		// front
		-h.x, -h.y, h.z,  0, 0, 1,
		h.x, -h.y, h.z,  0, 0, 1,
		h.x, h.y, h.z,  0, 0, 1,
		-h.x, h.y, h.z,  0, 0, 1,
		// right
		h.x, -h.y, h.z,  1, 0, 0,
		h.x, -h.y, -h.z,  1, 0, 0,
		h.x, h.y, -h.z,  1, 0, 0,
		h.x, h.y, h.z,  1, 0, 0,
		// top
		-h.x, h.y, h.z,  0, 1, 0,
		h.x, h.y, h.z,  0, 1, 0,
		h.x, h.y, -h.z,  0, 1, 0,
		-h.x, h.y, -h.z,  0, 1, 0,
		// bottom
		-h.x, -h.y, -h.z,  0, -1, 0,
		h.x, -h.y, -h.z,  0, -1, 0,
		h.x, -h.y, h.z,  0, -1, 0,
		-h.x, -h.y, h.z,  0, -1, 0,
		// back
		h.x, -h.y, -h.z,  0, 0, -1,
		-h.x, -h.y, -h.z,  0, 0, -1,
		-h.x, h.y, -h.z,  0, 0, -1,
		h.x, h.y, -h.z,  0, 0, -1,
		// left
		-h.x, -h.y, -h.z,  -1, 0, 0,
		-h.x, -h.y, h.z,  -1, 0, 0,
		-h.x, h.y, h.z,  -1, 0, 0,
		-h.x, h.y, -h.z,  -1, 0, 0
	};

	unsigned indices[] = {
		0, 1, 2,  2, 3, 0,
		4, 5, 6,  6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	mesh m = mesh{verts, sizeof(verts), indices, 2*6*3};
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 3*sizeof(GL_FLOAT)});
	return m;
}

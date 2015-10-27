#include "volumes.hpp"
#include <vector>
#include <cmath>
#include <GL/glew.h>

using std::vector;
using glm::vec3;
using gl::mesh;
using gl::attribute;
using gl::render_primitive;

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

mesh make_disk(float r, unsigned segments)
{
	float angle = 2.0*M_PI / (float)segments;

	vector<vec3> verts;
	verts.reserve(2*(segments+2));  // position:3, normal:3
	verts.push_back(vec3{0,0,0});
	verts.push_back(vec3{0,1,0});
	for (int i = 0; i < segments+1; ++i)
	{
		verts.push_back(vec3{r * cos(i*angle), 0, r * sin(i*angle)});
		verts.push_back(vec3{0,1,0});
	}

	vector<unsigned> indices;
	indices.reserve(segments+2);  // triangle-fan
	indices.push_back(0);
	for (unsigned i = segments+1; i > 0; --i)
		indices.push_back(i);

	mesh m = mesh(verts.data(), verts.size()*2*sizeof(vec3), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*4, 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*4, 3*4});
	m.draw_mode(render_primitive::triangle_fan);
	return m;
}

mesh make_open_cylinder(float r, float h, unsigned segments)
{
	float angle = 2.0*M_PI / (float)segments;

	// body
	vector<vec3> verts;  // position:3, normal:3
	for (int i = 0; i < segments; ++i)  // bottom
	{
		vec3 n = vec3{cos(i*angle), 0, sin(i*angle)};
		verts.push_back(r*n);
		verts.push_back(n);
	}

	int size = verts.size();  // top
	for (int i = 0; i < size; i += 2)
	{
		verts.push_back(verts[i]+vec3{0,h,0});
		verts.push_back(verts[i+1]);
	}

	vector<unsigned> indices;  // TODO: use triangle-strip
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(i+1);
		indices.push_back(i);
		indices.push_back(segments+i);
		indices.push_back(segments+i);
		indices.push_back(segments+i+1);
		indices.push_back(i+1);
	}
	indices.push_back(0);
	indices.push_back(segments-1);
	indices.push_back(segments+segments-1);
	indices.push_back(segments+segments-1);
	indices.push_back(segments);
	indices.push_back(0);

	mesh m = mesh(verts.data(), verts.size()*sizeof(vec3), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 3*sizeof(GL_FLOAT)});
	return m;
}

mesh make_cylinder(float r, float h, unsigned segments)
{
	float angle = 2.0*M_PI / (float)segments;

	// body
	vector<vec3> verts;  // position:3, normal:3
	for (int i = 0; i < segments; ++i)  // bottom
	{
		vec3 n = vec3{cos(i*angle), 0, sin(i*angle)};
		verts.push_back(r*n);
		verts.push_back(n);
	}

	int size = verts.size();  // top
	for (int i = 0; i < size; i += 2)
	{
		verts.push_back(verts[i]+vec3{0,h,0});
		verts.push_back(verts[i+1]);
	}

	vector<unsigned> indices;
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(i+1);
		indices.push_back(i);
		indices.push_back(segments+i);
		indices.push_back(segments+i);
		indices.push_back(segments+i+1);
		indices.push_back(i+1);
	}
	indices.push_back(0);
	indices.push_back(segments-1);
	indices.push_back(segments+segments-1);
	indices.push_back(segments+segments-1);
	indices.push_back(segments);
	indices.push_back(0);

	// bottom disk
	verts.push_back(vec3{0,0,0});
	verts.push_back(vec3{0,-1,0});
	for (int i = 0; i < 2*segments; i += 2)
	{
		verts.push_back(verts[i]);
		verts.push_back(vec3{0,-1,0});
	}

	unsigned off = 2*segments;
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(off);
		indices.push_back(off+1+i);
		indices.push_back(off+1+i+1);
	}
	indices.push_back(off);
	indices.push_back(off+segments);
	indices.push_back(off+1);

	// top disk
	verts.push_back(vec3{0,h,0});
	verts.push_back(vec3{0,1,0});
	for (int i = 0; i < 2*segments; i += 2)
	{
		verts.push_back(verts[2*segments+i]);
		verts.push_back(vec3{0,1,0});
	}

	off = 3*segments+1;
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(off+1+i+1);
		indices.push_back(off+1+i);
		indices.push_back(off);
	}
	indices.push_back(off+1);
	indices.push_back(off+segments);
	indices.push_back(off);

	mesh m = mesh(verts.data(), verts.size()*sizeof(vec3), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GL_FLOAT), 3*sizeof(GL_FLOAT)});
	return m;
}

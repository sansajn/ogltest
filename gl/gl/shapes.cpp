#include "shapes.hpp"
#include <vector>
#include <cmath>
#include <GL/glew.h>

using std::vector;
using glm::vec2;
using glm::vec3;
using gl::mesh;
using gl::attribute;
using gl::render_primitive;

namespace gl {

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
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
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
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
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
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
	return m;
}

mesh make_sphere(float r, unsigned hsegments, unsigned vsegments)
{
	float dp = M_PI / (float)vsegments;  // delta phi
	float dt = 2.0*M_PI / (float)hsegments;  // delta theta

	vector<vec3> verts;
	for (int i = 0; i < vsegments+1; ++i)
	{
		float phi = i*dp;
		for (int j = 0; j < hsegments; ++j)
		{
			float theta = j*dt;
			vec3 n = vec3{sin(phi)*sin(theta), cos(phi), sin(phi)*cos(theta)};
			verts.push_back(r*n);
			verts.push_back(n);
		}
	}

	vector<unsigned> inds;
	for (int i = 0; i < vsegments; ++i)
	{
		int roff = i*hsegments;
		for (int j = 0; j < hsegments-1; ++j)
		{
			inds.push_back(roff+j+hsegments);
			inds.push_back(roff+j+hsegments+1);
			inds.push_back(roff+j+1);
			inds.push_back(roff+j+1);
			inds.push_back(roff+j);
			inds.push_back(roff+j+hsegments);
		}
		inds.push_back(roff+hsegments-1+hsegments);
		inds.push_back(roff+hsegments);
		inds.push_back(roff);
		inds.push_back(roff);
		inds.push_back(roff+hsegments-1);
		inds.push_back(roff+hsegments-1+hsegments);
	}

	mesh m = mesh(verts.data(), verts.size()*sizeof(vec3), inds.data(), inds.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
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
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
	return m;
}

mesh make_cone(float r, float h, unsigned segments)
{
	float angle = 2.0*M_PI / (float)segments;

	// body
	vector<vec3> verts;  // position:3, normal:3
	for (int i = 0; i < segments; ++i)  // bottom disk
	{
		vec3 n = vec3{cos(i*angle), 0, sin(i*angle)};
		verts.push_back(r*n);
		verts.push_back(n);
	}

	verts.push_back(vec3{0,h,0});  // top peak
	verts.push_back(vec3{0,1,0});

	vector<unsigned> indices;
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(i+1);
		indices.push_back(i);
		indices.push_back(segments);
	}
	indices.push_back(0);
	indices.push_back(segments-1);
	indices.push_back(segments);

	// TODO: smooth normals

	// bottom disk
	verts.push_back(vec3{0,0,0});
	verts.push_back(vec3{0,-1,0});
	for (int i = 0; i < 2*segments; i += 2)
	{
		verts.push_back(verts[i]);
		verts.push_back(vec3{0,-1,0});
	}

	unsigned off = segments+1;
	for (int i = 0; i < segments-1; ++i)
	{
		indices.push_back(off);
		indices.push_back(off+1+i);
		indices.push_back(off+1+i+1);
	}
	indices.push_back(off);
	indices.push_back(off+segments);
	indices.push_back(off+1);

	mesh m = mesh(verts.data(), verts.size()*sizeof(vec3), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 0});
	m.append_attribute(attribute{2, 3, GL_FLOAT, (3+3)*sizeof(GLfloat), 3*sizeof(GLfloat)});
	return m;
}

mesh make_quad_xy()
{
	return make_quad_xy(glm::vec2(-1,-1), 2.0f);
}

mesh make_unit_quad_xy()
{
	return make_quad_xy(glm::vec2{0,0}, 1);
}

mesh make_quad_xy(glm::vec2 const & origin, float size)
{
	std::vector<vertex> verts{
		{glm::vec3(origin, 0), glm::vec2(0,0), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(size, 0), 0), glm::vec2(1,0), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(size, size), 0), glm::vec2(1,1), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(0, size), 0), glm::vec2(0,1), glm::vec3(0,0,1)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_quad_xz()
{
	return make_quad_xz(glm::vec2(-1,-1), 2.0f);
}

mesh make_quad_xz(glm::vec2 const & origin, float size)
{
	glm::vec2 const & o = origin;
	std::vector<vertex> verts{
		{glm::vec3(o.x, 0, -o.y), glm::vec2(0,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, -o.y), glm::vec2(1,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, -(o.y + size)), glm::vec2(1,1), glm::vec3(0,1,0)},
		{glm::vec3(o.x, 0, -(o.y + size)), glm::vec2(0,1), glm::vec3(0,1,0)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_quad_zy()
{
	return make_quad_zy(vec2{-1,-1}, 2.0f);
}

mesh make_quad_zy(vec2 const & origin, float size)
{
	vec2 const & o = origin;

	vector<vertex> verts{
		{vec3{0, o.y, o.x}, vec2{0,0}, vec3{1,0,0}},
		{vec3{0, o.y, o.x + size}, vec2{1,0}, vec3{1,0,0}},
		{vec3{0, o.y + size, o.x + size}, vec2{1,1}, vec3{1,0,0}},
		{vec3{0, o.y + size, o.x}, vec2{0,1}, vec3{1,0,0}}
	};

	vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xy(glm::vec3 const & origin, float size, unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float py = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(origin.x + size*px, origin.y + size*py, origin.z), glm::vec2(px, py), glm::vec3(0,0,1));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xy(unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float py = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(px, py, 0), glm::vec2(px, py), glm::vec3(0,0,1));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xz(unsigned w, unsigned h, float size)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float pz = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(size*px, 0, -size*pz), glm::vec2(px, pz), glm::vec3(0,1,0));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xz(glm::vec3 const & origin, float size, unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float pz = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(origin + glm::vec3(size*px, 0, -size*pz), glm::vec2(px, pz), glm::vec3(0,1,0));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_axis()
{
	vector<float> vertices{  // position, color
		0,0,0, 1,0,0,
		1,0,0, 1,0,0,
		0,0,0, 0,1,0,
		0,1,0, 0,1,0,
		0,0,0, 0,0,1,
		0,0,1, 0,0,1};

	vector<unsigned> indices{0,1, 2,3, 4,5};

	mesh m(vertices.data(), vertices.size()*sizeof(float), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, 6*sizeof(GLfloat)});  // position
	m.append_attribute(attribute{1, 3, GL_FLOAT, 6*sizeof(GLfloat), 3*sizeof(GLfloat)});  // color
	m.draw_mode(render_primitive::lines);
	return m;
}

}  // gl

#include "plane.h"
#include <vector>

#include <glm/glm.hpp>

#include <iostream>


namespace gl {

using std::vector;

template <typename It, typename T>
void assign_and_inc(It & it, T const & v)
{
	*it = v;
	++it;
}

inline int num_plane_indices(int w, int h) 
{ 
	return (2 * (w - 1) + 2) * (h - 1) + h;  // +h for primitive restart indices
}


vector<glm::vec3> generate_plane_vertices(int w, int h)
{
	vector<glm::vec3> verts(w*h);
	auto it = verts.begin();

	float w_side = 1.0f/(w-1);
	float h_side = 1.0f/(h-1);

	for (int j = 0; j < h; ++j)
	{
		float z = j*h_side;
		for (int i = 0; i < w; ++i)
		{
			float x = i*w_side;
			assign_and_inc(it, glm::vec3(x, 0, z));
		}
	}

	return verts;
}

vector<GLuint> generate_plane_indices(int w, int h)
{
	vector<GLuint> inds(num_plane_indices(w, h));
	auto it = inds.begin();

	for (int j = 1; j < h; ++j)
	{
		int v1 = j*w;
		int v0 = v1-w;

		assign_and_inc(it, v0);
		assign_and_inc(it, v1);

		for (int i = 1; i < w; ++i)
		{
			assign_and_inc(it, v0+i);  // v2
			assign_and_inc(it, v1+i);  // v3
		}

		assign_and_inc(it, -1);  // primitive restart-index
	}

	return inds;
}

vbo_plane::vbo_plane(int w, int h)
	: _w(w), _h(h), _vao(0)
{
	vector<glm::vec3> verts = generate_plane_vertices(w, h);
	vector<GLuint> inds = generate_plane_indices(w, h);

	// supni ich do pamete
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	GLuint bufs[2];
	glGenBuffers(2, bufs);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(GLuint), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_plane::render()
{
	glBindVertexArray(_vao);
	glPrimitiveRestartIndex(-1);
	glEnable(GL_PRIMITIVE_RESTART);
	glDrawElements(GL_TRIANGLE_STRIP, num_plane_indices(_w, _h), GL_UNSIGNED_INT, 0);
	glDisable(GL_PRIMITIVE_RESTART);
}


}  // gl

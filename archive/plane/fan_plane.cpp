#include "fan_plane.h"
#include <vector>
#include <glm/glm.hpp>

namespace gl {

using std::vector;

vector<glm::vec3> generate_plane_verts(int w, int h)
{
	int nverts = w*h;
	vector<glm::vec3> verts(nverts);
	float w_square = 1.0f/(w-1);
	float h_square = 1.0f/(h-1);

	for (int j = 0; j < h; ++j)
	{
		int r0 = j*w;
		for (int i = 0; i < w; ++i)
			verts[r0+i] = glm::vec3(i*w_square, 0, j*h_square);
	}

	return verts;
}

inline int num_plane_indices(int w, int h)
{
	return (w-1)*(h-1)*5+(w*h/2);  // (((w-1)/2.0 * (h-1)/2.0) * 10) + w*h/2
}

vector<GLuint> generate_plane_indices(int w, int h)
{
	vector<GLuint> inds(num_plane_indices(w, h));
	GLuint * ind_ptr = &inds[0];

	for (int i = 1; i < h; i+=2)
	{
		int r0 = i*w;
		for (int j = 1; j < w; j+=2)
		{
			int vc = r0+j;  // center
			int v0 = vc-w-1;
			int v1 = v0+1;
			int v2 = v1+1;
			int v3 = vc-1;
			int v4 = vc+1;
			int v5 = v3+w;
			int v6 = v5+1;
			int v7 = v6+1;
			*(ind_ptr++) = vc;
			*(ind_ptr++) = v0;
			*(ind_ptr++) = v1;
			*(ind_ptr++) = v2;
			*(ind_ptr++) = v4;
			*(ind_ptr++) = v7;
			*(ind_ptr++) = v6;
			*(ind_ptr++) = v5;
			*(ind_ptr++) = v3;
			*(ind_ptr++) = v0;

			*(ind_ptr++) = -1;  // primitive restart-index
		}
	}

	return inds;
}

vbo_fan_plane::vbo_fan_plane(int w, int h)
	: _w(w), _h(h), _vao(0)
{
	vector<glm::vec3> verts = generate_plane_verts(w, h);
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

vbo_fan_plane::~vbo_fan_plane()
{}

void vbo_fan_plane::render()
{
	glBindVertexArray(_vao);
	glPrimitiveRestartIndex(-1);
	glEnable(GL_PRIMITIVE_RESTART);
	glDrawElements(GL_TRIANGLE_FAN, num_plane_indices(_w, _h), GL_UNSIGNED_INT, 0);
	glDisable(GL_PRIMITIVE_RESTART);
}

}

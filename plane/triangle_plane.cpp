#include "triangle_plane.h"
#include <vector>

#include <glm/glm.hpp>

#include <iostream>


namespace gl {

using std::vector;

vbo_triangle_plane::vbo_triangle_plane(int w, int h)
	: _vao(0), _w(w), _h(h)
{
	// vrcholy
	int nverts = w*h;
	float w_square = 1.0f/(w-1);
	float h_square = 1.0f/(h-1);
	vector<glm::vec3> verts(nverts);	
	for (int j = 0; j < h; ++j)
	{
		int r0 = j*w;
		for (int i = 0; i < w; ++i)
			verts[r0+i] = glm::vec3(i*w_square, 0, j*h_square);
	}

	// indexy
	int ninds = (w-1)*(h-1)*2*3;
	vector<GLuint> inds(ninds);
	GLuint * ind_ptr = &inds[0];
	for (int j = 1; j < h; ++j)
	{
		int r0 = j*w;
		for (int i = 1; i < w; ++i)			
		{
			int v3 = r0+i;
			int v2 = v3-1;
			int v1 = v3-w;
			int v0 = v1-1;
			*(ind_ptr++) = v0;
			*(ind_ptr++) = v2;
			*(ind_ptr++) = v1;
			*(ind_ptr++) = v1;
			*(ind_ptr++) = v2;
			*(ind_ptr++) = v3;
		}
	}

	// supni ich do pamete
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	GLuint bufs[2];
	glGenBuffers(2, bufs);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, nverts*sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ninds*sizeof(GLuint), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_triangle_plane::render()
{
	glBindVertexArray(_vao);
	int ninds = (_w-1)*(_h-1)*2*3;
	glDrawElements(GL_TRIANGLES, ninds, GL_UNSIGNED_INT, 0);
}


}  // gl

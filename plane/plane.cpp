#include "plane.h"

namespace gl {

vbo_plane::vbo_plane(int w, int h)
	: _vao(0), _w(w), _h(h)
{
	// vrcholy
	int nverts = w*h;
	float w_side = 1.0f/w;
	float h_side = 1.0f/h;
	vector<glm::vec3> v(nverts);
	for (int j = 0; j < h; ++j)
	{
		int r0 = j*w;
		for (int i = 0; i < w; ++i)
			v[r0+i] = glm::vec3(i*w_side, 0, j*h_side);
	}

	// normals

	// indexy
	int ninds = ((w*h)-2)*3;
	vector<GLuint> indices(ninds);
	int * ind_ptr = &indices[0];
	for (int j = 1; j < h; ++j)
	{
		int r0 = j*w;
		for (int i = 1; i < w; ++i)			
		{
			int v1 = r0+i;
			int v0 = v1-1;
			int v3 = v1+w;
			int v2 = v3-1;
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

	// verts
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, nverts*sizeof(glm::vec3), &v[0], 
		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ninds*sizeof(GLuint), &indices[0], 
		GL_STATIC_BUFFER);

	glBindVertexArray(0);
}

void vbo_plane::render()
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, (_w*_h)-2, GL_UNSIGNED_INT, 0);
}


}  // gl

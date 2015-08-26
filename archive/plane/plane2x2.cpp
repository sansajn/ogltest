#include "plane2x2.h"

#include <vector>

#include <glm/glm.hpp>

namespace gl {

using std::vector;

vbo_plane2x2::vbo_plane2x2()
	: _vao(0)
{
	vector<glm::vec3> verts(4);
	verts[0] = glm::vec3(0, 0, 0);
	verts[1] = glm::vec3(1, 0, 0);
	verts[2] = glm::vec3(0, 0, 1);
	verts[3] = glm::vec3(1, 0, 1);

	vector<int> inds = {0, 2, 1, 1, 2, 3};

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	GLuint bufs[2];
	glGenBuffers(2, bufs);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(int), &inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_plane2x2::render()
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

}  // gl

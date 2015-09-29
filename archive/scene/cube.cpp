#include "cube.h"

namespace gl {

vbo_cube::vbo_cube()
	: _vao(0)
{
	float side = 1.0f;
	float side2 = side/2.0f;

	float v[24*3] = {
		// Front
		-side2, -side2, side2,
		 side2, -side2, side2,
		 side2,  side2, side2,
		-side2,  side2, side2,
		// Right
		 side2, -side2, side2,
		 side2, -side2, -side2,
		 side2,  side2, -side2,
		 side2,  side2, side2,
		// Back
		-side2, -side2, -side2,
		-side2,  side2, -side2,
		 side2,  side2, -side2,
		 side2, -side2, -side2,
		// Left
		-side2, -side2, side2,
		-side2,  side2, side2,
		-side2,  side2, -side2,
		-side2, -side2, -side2,
		// Bottom
		-side2, -side2, side2,
		-side2, -side2, -side2,
		 side2, -side2, -side2,
		 side2, -side2, side2,
		// Top
		-side2,  side2, side2,
		 side2,  side2, side2,
		 side2,  side2, -side2,
		-side2,  side2, -side2
	};

	float n[24*3] = {
		// Front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		// Back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		// Left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		// Bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		// Top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint el[] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	glGenVertexArrays(1, &_vao);  // vytvor 1 vertex-array
	glBindVertexArray(_vao);

	unsigned int bufs[3];
	glGenBuffers(3, bufs);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), n, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), el,
		GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_cube::render()
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

}  // gl

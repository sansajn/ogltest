/* nakresli trojuholnik pomocou VBO */
#include <iostream>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "src/program.h"
#include "buffer.h"


int const WIDTH = 800;
int const HEIGHT = 600;

void display();
void reshape(int w, int h);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

gl::program prog;
GLuint vao = -1;
GLuint positionID = -1;
GLuint colorID = -1;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glutSwapBuffers();
}


int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("triangle using vao");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "GLEW error";
		return 1;
	}
	glGetError();  // eat error

	GLfloat vertices[] = {
		-.5f, -.5f, .0f,
		.5f, -.5f, .0f,
		.0f, .5f, .0f};

	GLfloat colors[] = {
		1.0f, .0f, .0f, 1.0f,
		.0f, 1.0f, .0f, 1.0f,
		.0f, .0f, 1.0f, 1.0f};

	GLfloat verts_colors[] = {
		-.5f, -.5f, .0f,   1.0f,  .0f,  .0f, 1.0f,
		 .5f, -.5f, .0f,    .0f, 1.0f,  .0f, 1.0f,
		 .0f,  .5f, .0f,    .0f,  .0f, 1.0f, 1.0f
	};

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	assert(glGetError() == GL_NO_ERROR && "opengl error");

	gpubuffer triangle_buf;	
	triangle_buf.data(7*3*sizeof(float), verts_colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, triangle_buf.id());

	positionID = prog.attrib_location("s_vPosition");
	colorID = prog.attrib_location("s_vColor");

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 7*4, 0);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 7*4, BUFFER_OFFSET(3*4));

	prog.use();

	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}


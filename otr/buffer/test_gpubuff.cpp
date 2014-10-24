/* nakresli trojuholnik s pouzitim gpubuffer kde vrchol a farba su navzajom oddelene data */
#include <iostream>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "render/program.hpp"
#include "render/gpubuffer.h"

int const WIDTH = 800;
int const HEIGHT = 600;

void display();
void reshape(int w, int h);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

shader_program prog;
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

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	prog.use();
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	assert(glGetError() == GL_NO_ERROR && "opengl error");

	gpubuffer triangle_buf;
	triangle_buf.reserve(7*3*sizeof(GLfloat));
	triangle_buf.subdata(0, 3*3*sizeof(GLfloat), (GLvoid *)vertices);
	triangle_buf.subdata(3*3*sizeof(GLfloat), 3*4*sizeof(GLfloat), (GLvoid *)colors);
	triangle_buf.bind(GL_ARRAY_BUFFER);

	positionID = prog.attrib_location("s_vPosition");
	colorID = prog.attrib_location("s_vColor");

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(3*3*sizeof(GLfloat)));

	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}


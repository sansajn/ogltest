#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "program.h"

using std::cout;
using std::cerr;
using std::endl;

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();

GLuint vao;
gl::program prog;


void on_init()
{
	glm::vec3 verts[3] = {
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0), 
		glm::vec3(0, 1, 0)
	};

	GLuint inds[3] = {0, 1, 2};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint bufs[2];
	glGenBuffers(2, bufs);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, 3*sizeof(glm::vec3), &verts[0], 
		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(GLuint), &inds[0], 
		GL_STATIC_DRAW);

	glBindVertexArray(0);

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
}

void on_render()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

void on_close()
{
	glDeleteVertexArrays(1, &vao);
}

int main(int argc, char * argv[])
{
	int w = 800, h = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(w, h);
	glutCreateWindow("triangle test");

	init_glew();

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	on_init();

	GL_CHECK_ERRORS;

	prog.use();

	glm::mat4 project = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), 
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 vp = project*view;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 model(1.0f);
	glm::mat4 mvp = vp*model;
	prog.uniform("mvp", mvp);

	on_render();

	GL_CHECK_ERRORS;

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void init_glew()
{
	glewExperimental = GL_TRUE;  // inak vo verzii 1.8 padne na glGenVertexArrays()
	GLenum state = glewInit();
	if (state != GLEW_OK)
		cerr << "Error: " << glewGetErrorString(state) << endl;

	if (GLEW_VERSION_4_0)
		cout << "OpenGL 4.0 supported\n";

	glGetError();  // swallow error 1282
	GL_CHECK_ERRORS;
}


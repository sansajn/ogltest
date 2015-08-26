#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "program.h"
#include "plane.h"

using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();
void dump_hw_info();


int main(int argc, char * argv[])
{
	int w = 800, h = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(w, h);
	glutCreateWindow("vbo plane test");

	init_glew();

	dump_hw_info();

	glEnable(GL_DEPTH_TEST);	

	gl::program prog;
	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	prog.use();

	GL_CHECK_ERRORS;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 project = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 1, 1),
		glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 vp = project*view;

	gl::vbo_plane plane(16, 16);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 M(1);
	M = glm::translate(M, glm::vec3(-.5f, 0, -.5f));
	glm::mat4 MVP = vp*M;
	prog.uniform("mvp", MVP);
	prog.uniform("model_color", glm::vec4(1, 1, 1, 1));
	plane.render();

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

void dump_hw_info()
{
	cout << "\tUsing GLEW "<< glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

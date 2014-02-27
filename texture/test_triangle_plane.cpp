#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "src/program.h"
#include "src/triangle_plane.h"

#include "texture.h"

using std::cout;
using std::cerr;
using std::endl;

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();

GLuint texture = -1;
gl::program prog;
gl::vbo_triangle_plane * plane = nullptr;


void on_init()
{
	prog << "shader/plane.vs" << "shader/plane.fs";
	prog.link();
	prog.use();

	plane = new gl::vbo_triangle_plane(10, 10);
	assert(plane && "can't create plane object");
	
	// precitaj texturu
	gl::texture smiley("data/smiley.png");
	assert(smiley.loaded() && "can't laod 'data/smiley.png' image");

	// natiahni ju do opengl
	glActiveTexture(GL_TEXTURE0);
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, smiley.width(), smiley.height());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, smiley.width(), smiley.height(),
		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)smiley.data());

	prog.sampler_uniform("tex_sampler", 0);  // texture unit goes there (not a texture)
}

void on_render()
{
	glClearColor(.0f, .0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	plane->render();
}

void on_close()
{
	glDeleteTextures(1, &texture);
	delete plane;
}

int main(int argc, char * argv[])
{
	int w = 800, h = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(w, h);
	glutCreateWindow("simple texture");

	glutCloseFunc(on_close);

	init_glew();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	on_init();

	GL_CHECK_ERRORS;

	prog.use();

	glm::mat4 P = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), 
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 VP = P*V;

	glm::mat4 M(1.0f);
	glm::mat4 MVP = VP*M;
	prog.uniform("MVP", MVP);

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


#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>

#include "program.h"
#include "plane.h"
#include "scene.h"

using std::cout;
using std::cerr;
using std::endl;

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();

GLuint vao;
gl::program prog;
gl::vbo_plane * plane;  // nemozem este vytvorit, lebo glew nie je aninicializovany


void on_init()
{
/*
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
*/

	plane = new gl::vbo_plane(512, 512);

	// precitaj texturu
	int w = 0, h = 0, ch = 0;
	GLubyte * image = SOIL_load_image("smile.png", &w, &h, &ch, SOIL_LOAD_RGBA);

/*
	// flip y axis
	for (int j = 0; j*2 < h; ++j)
	{
		int h0 = j*w;
		int h0_inv = (h-1-j)*w;
		for (int i = w; i > 0; ++i)
			std::swap(image[h0++], image[h0_inv++]);
	}
*/

	// natiahni ju do opengl
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (void *)image);

	// poupratuj
	SOIL_free_image_data(image);
	
	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
}

void on_render()
{
/*
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
*/

	plane->render();
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
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	on_init();

	GL_CHECK_ERRORS;

	prog.use();

	glm::mat4 project = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 2, 0), glm::vec3(0, 0, 0), 
		glm::vec3(0.0f, 0.0f, -1.0f));
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


#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "src/program.h"
#include "texture.h"

using std::cout;
using std::cerr;
using std::endl;

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();

int const WIDTH = 800;
int const HEIGHT = 600;

GLuint vao = -1;
GLuint vbo = -1;
GLuint texture = -1;
shader_program prog;


void on_init()
{
//	prog << "shader/texture.vs" << "shader/texture.fs";
	prog << "shader/minimal.fs";
	prog.link();
	prog.use();
	
	glm::vec3 verts[6] = {
		glm::vec3(1, 1, 0),
		glm::vec3(-1, 1, 0),
		glm::vec3(-1, -1, 0),
		
		glm::vec3(1, 1, 0),
		glm::vec3(-1, -1, 0),
		glm::vec3(1, -1, 0)
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
		
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);

//	GLuint position_loc = prog.attrib_location("position");
//	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
//	glEnableVertexAttribArray(position_loc);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	// precitaj texturu
//	gl::texture smiley("data/smiley.png");
//	assert(smiley.loaded() && "can't laod 'data/smiley.png' image");

	// natiahni ju do opengl
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

// povodne
//	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, smiley.width(), smiley.height());
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, smiley.width(), smiley.height(),
//		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)smiley.data());

//	funkcne
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // treba nastavit, inak glTexImage2D nezobrazi nic
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, smiley.width(), smiley.height(), 0,
//		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)smiley.data());

	unsigned char data[16] = {0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0};
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // treba nastavit, inak glTexImage2D nezobrazi nic
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid *)data);

	uniform_variable u_sampler("sampler", prog);
	u_sampler = 0;  // texture unit goes there (not a texture)

	uniform_variable u_scale("scale", prog);
	u_scale = glm::vec2(1.0/WIDTH, 1.0/HEIGHT);
}

void on_render()
{
	glClearColor(.0f, .0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void on_close()
{
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("simple texture");

	glutCloseFunc(on_close);

	init_glew();

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	on_init();

	GL_CHECK_ERRORS;

	prog.use();

//	glm::mat4 MVP(1.0f);
//	uniform_variable u_mvp("MVP", prog);
//	u_mvp = MVP;

	on_render();

	GL_CHECK_ERRORS;

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void init_glew()
{
	glewExperimental = GL_TRUE;
	GLenum state = glewInit();
	if (state != GLEW_OK)
		cerr << "Error: " << glewGetErrorString(state) << endl;
	glGetError();  // swallow error 1282
	GL_CHECK_ERRORS;
}


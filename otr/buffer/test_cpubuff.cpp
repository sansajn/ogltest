// zozbrazí čierno červenú šachovnicu
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render/program.h"
#include "render/cpubuffer.h"

using std::cerr;
using std::endl;

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew();
void on_init();
void on_close();
void on_render();

GLuint vao = -1;
GLuint vbo = -1;
GLuint texture_id = -1;
shader_program prog;


void on_init()
{
	prog << "shader/texture.vs" << "shader/texture.fs";
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

	GLuint position_loc = prog.attrib_location("position");
	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_loc);
	
	// aktyvuj texturovaciu jednotku 0
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

//	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, smiley.width(), smiley.height());
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, smiley.width(), smiley.height(),
//		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)smiley.data());

//	funkcne
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // treba nastavit, inak glTexImage2D nezobrazi nic
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, smiley.width(), smiley.height(), 0,
//		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)smiley.data());

	unsigned char data[16] = {0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0};

	cpubuffer buf(data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid *)buf.data(0));

	uniform_variable u_tex("tex", prog);
	u_tex = 0;  // texture unit goes there (not a texture)
}

void on_render()
{
	glClearColor(.0f, .0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void on_close()
{
	glDeleteTextures(1, &texture_id);
	glDeleteBuffers(1, &vbo);
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
	glutCreateWindow("simple texture");
	glutCloseFunc(on_close);

	init_glew();
	on_init();

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_CHECK_ERRORS;

	prog.use();

	glm::mat4 P = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), 
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 VP = P*V;
	glm::mat4 M(1.0f);
	glm::mat4 MVP = VP*M;

	uniform_variable u_mvp("MVP", prog);
	u_mvp = MVP;

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

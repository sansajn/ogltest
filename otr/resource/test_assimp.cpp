/* nakresli trojuholnik pomocou VBO */
#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render/program.hpp"
#include "render/framebuffer.h"
#include "resource/assimp_loader.hpp"


int const WIDTH = 800;
int const HEIGHT = 600;


void display();
void reshape(int w, int h);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

shader_program prog;
GLuint vao = -1;
GLuint positionID = -1;
GLuint colorID = -1;
ptr<mesh_buffers> m;


void display()
{
	framebuffer & fb = framebuffer::default_fb();
	fb.clear(true, true);
	m->draw();
	glutSwapBuffers();
}


int main(int argc, char * argv[])
{
	std::string model_name = "monkey_color.ply";
	if (argc > 1)
		model_name = argv[1];

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("mesh loader test");

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

	framebuffer::default_fb().depth_test(true);

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog.use();

	glm::mat4 P = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
	glm::mat4 MVP = P*V*glm::mat4(1.0);

	uniform_variable u_mvp("mvp", prog);
	u_mvp = MVP;

	assimp_loader loader;
	m = loader.load(std::string("meshes/") + model_name);

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

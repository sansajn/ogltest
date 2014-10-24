/* nakresli trojuholnik pomocou mesh objektu */
#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "render/program.hpp"
#include "render/mesh.h"
#include "render/framebuffer.h"


int const WIDTH = 800;
int const HEIGHT = 600;


struct P3_C
{
	float x, y, z;
	float r, g, b, a;
	P3_C(float x, float y, float z, float r, float g, float b, float a)
		: x(x), y(y), z(z), r(r), g(g), b(b), a(a)
	{}
};

void display();
void reshape(int w, int h);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

shader_program prog;
GLuint vao = -1;
GLuint positionID = -1;
GLuint colorID = -1;
mesh<P3_C, unsigned int> * m = nullptr;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	framebuffer & fb = framebuffer::default_fb();
	fb.draw(prog, *m);
	glutSwapBuffers();
}


int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("mesh test");

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

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog.use();

	positionID = prog.attrib_location("s_vPosition");
	colorID = prog.attrib_location("s_vColor");

	assert(positionID != -1 && colorID != -1 && "unknown attribute location");

	m = new mesh<P3_C, unsigned int>(mesh_mode::triangles, mesh_usage::GPU_STATIC);
	m->append_attribute_type(positionID, 3, attribute_type::f32);
	m->append_attribute_type(colorID, 4, attribute_type::f32);
	m->append_vertex(P3_C(-.5f, -.5f, .0f,   1.0f,  .0f,  .0f, 1.0f));
	m->append_vertex(P3_C( .5f, -.5f, .0f,    .0f, 1.0f,  .0f, 1.0f));
	m->append_vertex(P3_C( .0f,  .5f, .0f,    .0f,  .0f, 1.0f, 1.0f));

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

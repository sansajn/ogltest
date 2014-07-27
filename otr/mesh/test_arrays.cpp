/* nakresli trojuholnik pomocou mesh_buffers (gl-draw-arrays) */
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "render/program.h"
#include "render/gpubuffer.h"
#include "render/meshbuffers.h"

int const WIDTH = 800;
int const HEIGHT = 600;

void display();
void reshape(int w, int h);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

shader_program prog;
GLuint vao = -1;
GLuint positionID = -1;
GLuint colorID = -1;
mesh_buffers * mesh = nullptr;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	mesh->draw();
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
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog.use();

	positionID = prog.attrib_location("s_vPosition");
	colorID = prog.attrib_location("s_vColor");

	ptr<gpubuffer> verts_buf = std::make_shared<gpubuffer>();
	verts_buf->data(3*3*sizeof(GLfloat), (GLvoid *)vertices, buffer_usage::STATIC_DRAW);

	ptr<gpubuffer> color_buf = std::make_shared<gpubuffer>();
	color_buf->data(4*3*sizeof(GLfloat), (GLvoid *)colors, buffer_usage::STATIC_DRAW);

	mesh = new mesh_buffers();
	mesh->append_attribute(
		std::make_shared<attribute_buffer>(positionID, 3, attribute_type::A32F, verts_buf));
	mesh->append_attribute(
		std::make_shared<attribute_buffer>(colorID, 4, attribute_type::A32F, color_buf));
	mesh->nvertices = 3;
	mesh->mode = mesh_mode::TRIANGLES;

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

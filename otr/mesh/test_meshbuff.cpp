/* test mesh_buffers objektu (bufer data su prekladane) */
#include <iostream>
#include <cassert>
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
GLuint position_id = -1;
GLuint color_id = -1;
mesh_buffers * m;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	m->draw();
	glutSwapBuffers();
}


int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("test mesh_buffers");

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

	GLfloat verts_colors[] = {
		-.5f, -.5f, .0f,   1.0f,  .0f,  .0f, 1.0f,
		 .5f, -.5f, .0f,    .0f, 1.0f,  .0f, 1.0f,
		 .0f,  .5f, .0f,    .0f,  .0f, 1.0f, 1.0f
	};

	prog << "shader/simple.vs" << "shader/simple.fs";
	prog.link();
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	assert(glGetError() == GL_NO_ERROR && "opengl error");

	ptr<gpubuffer> triangle_buf = make_ptr<gpubuffer>();
	triangle_buf->data(7*3*sizeof(float), verts_colors, buffer_usage::STATIC_DRAW);

	position_id = prog.attrib_location("s_vPosition");
	color_id = prog.attrib_location("s_vColor");

	m = new mesh_buffers();
	m->append_attribute(make_ptr<attribute_buffer>(position_id, 3, attribute_type::f32, triangle_buf, 7*4));
	m->append_attribute(make_ptr<attribute_buffer>(color_id, 4, attribute_type::f32, triangle_buf, 7*4, 3*4));
	m->mode = mesh_mode::triangles;
	m->nvertices = 3;

	prog.use();

	glutMainLoop();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}


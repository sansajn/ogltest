// vykresli texturu
#include <string>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "program.hpp"
#include "utils.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "math.hpp"

using std::string;

void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	string shader_code = read_file("texture.glsl");
	shader::program prog(make_ptr<shader::module>(shader_code));

	mesh m("plane_xy.obj");
	texture t("bricks.jpg");

	transform r;
	r.uniform_scale(0.5f);
	r.rotate(glm::vec3(0, 0, 1), 0.2);

	ptr<shader::uniform> MVP = prog.uniform_variable("MVP");
	ptr<shader::uniform> diffuse = prog.uniform_variable("diffuse");

	prog.use();

	t.bind(0);

	*MVP = r.transformation();
	*diffuse = 0;  // texture unit

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	m.draw();
	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL triangle");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error

	glViewport(0, 0, 800, 600);
}

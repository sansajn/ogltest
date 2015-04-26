// na obrazovku vykresli texturu pomociu triedy texture
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "program.hpp"
#include "texture.hpp"
#include "mesh.hpp"

using std::vector;
using std::pair;
using std::make_pair;

void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	shader::program prog("assets/shaders/texrender.glsl");
	texture2d tex("assets/textures/bricks.png");
	mesh texframe = make_quad_xy();

	// render
	prog.use();
	tex.bind(0);
	prog.uniform_variable("tex", 0);
	texframe.draw();

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL texture demo");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

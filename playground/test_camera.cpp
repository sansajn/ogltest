// nakresí zuzanu
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <boost/format.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "program.hpp"
#include "mesh.hpp"
#include "math.hpp"

using std::string;

void init(int argc, char * argv[]);

class game
{
public:
	static game & ref();

	void run();
	void input(unsigned char key);
	void update() {}
	void render();

	static void keyboard(unsigned char key, int x, int y) {game::ref().input(key);}

	static void display() {
		game::ref().render();
		glutSwapBuffers();
		glutPostRedisplay();
	}

private:
	game() {}
	void init();

	shader::program * _prog;
	mesh * _mesh;
	transform _transf;
};


int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	game::ref().run();

	return 0;
}

game & game::ref()
{
	static game * g = nullptr;
	if (!g)
	{
		g = new game();
		g->init();
	}
	return *g;
}

void game::init()
{
	_prog = new shader::program("simple.glsl");
	_mesh = new mesh("monkey3.obj");
	_transf.uniform_scale(0.5f);
}

void game::run()
{
	glutMainLoop();
}

void game::input(unsigned char key)
{}

void game::render()
{
	_prog->use();

	ptr<shader::uniform> MVP = _prog->uniform_variable("MVP");
	*MVP = _transf.transformation();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_mesh->draw();
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

	glutDisplayFunc(game::display);
	glutKeyboardFunc(game::keyboard);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

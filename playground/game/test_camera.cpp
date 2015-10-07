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
#include <glm/gtc/matrix_transform.hpp>
#include "program.hpp"
#include "mesh.hpp"
#include "math.hpp"
#include "camera.hpp"
#include "texture.hpp"

using std::string;
using std::cout;

void init(int argc, char * argv[]);

class game
{
public:
	static game & ref();

	enum class input_type
	{
		motion,
		keyboard
	};

	void run();
	void input(input_type type, unsigned char key, int x, int y);
	void update() {}
	void render();

	static void display();
	static void keyboard(unsigned char key, int x, int y);
	static void motion(int x, int y);

private:
	game() : _cam(70.0f, 800.0f/600.0f, 0.01f, 1000.0f) {}
	void init();

	shader::program * _prog;
	mesh * _mesh;
	texture * _diffuse;
	transform _transf;
	camera _cam;

	static bool fps_mode;
};

bool game::fps_mode = false;


void game::init()
{
	_prog = new shader::program("texture.glsl");
	_mesh = new mesh("monkey3.obj");
	_diffuse = new texture("bricks.jpg");
	_cam.transformation.position = glm::vec3(0, 0, 5);
}

void game::input(input_type type, unsigned char key, int x, int y)
{
	float const movement = 0.1f;
	float const angular_movement = 0.1f;

	if (type == input_type::keyboard)
	{
		switch (key)
		{
			case 'a':
				_cam.transformation.position -= _cam.right() * 0.1f;
				break;

			case 'd':
				_cam.transformation.position += _cam.right() * 0.1f;
				break;

			case 'w':
				_cam.transformation.position -= _cam.forward() * 0.1f;
				break;

			case 's':
				_cam.transformation.position += _cam.forward() * 0.1f;
				break;

			case ' ':  // space
				fps_mode = !fps_mode;

				if (fps_mode)
				{
					glutSetCursor(GLUT_CURSOR_NONE);
					glutWarpPointer(800/2, 600/2);
				}
				else
					glutSetCursor(GLUT_CURSOR_INHERIT);

				break;
		}
	}

	if (type == input_type::motion && fps_mode)
	{
		int dx = x - 800/2;
		int dy = y - 600/2;

		if (dx != 0)
		{
			float angle = angular_movement * dx;
			_cam.transformation.rotate(glm::vec3(0,1,0), -angle);
		}

		if (dy != 0)
		{
			float angle = angular_movement * dy;
			_cam.transformation.rotate(_cam.right(), -angle);
		}

		if (dx != 0 || dy != 0)
			glutWarpPointer(800/2, 600/2);
	}
}

void game::render()
{
	_prog->use();
	_diffuse->bind(0);

	ptr<shader::uniform> MVP = _prog->uniform_variable("MVP");
	*MVP = _cam.view_projection() * _transf.transformation();

	ptr<shader::uniform> diffuse = _prog->uniform_variable("diffuse");
	*diffuse = 0;

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
	glutPassiveMotionFunc(game::motion);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error

	glEnable(GL_DEPTH_TEST);
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

void game::run()
{
	glutMainLoop();
}

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	game::ref().run();

	return 0;
}

void game::display()
{
	game::ref().render();
	glutSwapBuffers();
	glutPostRedisplay();
}

void game::keyboard(unsigned char key, int x, int y)
{
	game::ref().input(input_type::keyboard, key, x, y);
}

void game::motion(int x, int y)
{
	game::ref().input(input_type::motion, '\0', x, y);
}

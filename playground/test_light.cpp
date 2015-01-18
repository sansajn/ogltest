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
#include "input.hpp"

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
	void input();
	void update() {}
	void render();

	void _input(input_type type, unsigned char key, int x, int y);

	static void display();
	static void motion(int x, int y);
	static void idle();

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
	_prog = new shader::program("directional_light.glsl");
	_mesh = new mesh("monkey3.obj");
	_diffuse = new texture("bricks.jpg");
	_cam.transformation.position = glm::vec3(0, 0, 5);
}

void game::render()
{
	_prog->use();
	_diffuse->bind(0);

	ptr<shader::uniform> M = _prog->uniform_variable("M");
	*M = _transf.transformation();

	ptr<shader::uniform> MVP = _prog->uniform_variable("MVP");
	*MVP = _cam.view_projection() * _transf.transformation();

	ptr<shader::uniform> diffuse = _prog->uniform_variable("diffuse");
	*diffuse = 0;

	ptr<shader::uniform> eye_position = _prog->uniform_variable("eye_pos");
	*eye_position = _cam.transformation.position;

	// light
	ptr<shader::uniform> light_direction = _prog->uniform_variable("light_direction");
//	transform light_transf;
//	light_transf.rotation = glm::quat(-45, glm::vec3(1,0,0));
//	glm::vec4 light_dir = light_transf.transformation()[2];
//	*light_direction = glm::vec3(light_dir.x, light_dir.y, light_dir.z);  // forward of transformation
//	*light_direction = glm::vec3(-1, -1, -1);

	ptr<shader::uniform> light_color = _prog->uniform_variable("light_color");
//	*light_color = glm::vec3(1,1,1);

	ptr<shader::uniform> light_intensity = _prog->uniform_variable("light_intensity");
//	*light_intensity = 0.4f;

//	// material
//	ptr<shader::uniform> specular_intensity = _prog->uniform_variable("MATERIAL.specular_intensity");
//	*specular_intensity = 1.0f;

//	ptr<shader::uniform> specular_power = _prog->uniform_variable("MATERIAL.specular_power");
//	*specular_power = 8.0f;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_mesh->draw();
}

void game::input()
{
	float const movement = 0.1f;

	if (input::key('a'))
		_cam.transformation.position -= _cam.right() * movement;

	if (input::key('d'))
		_cam.transformation.position += _cam.right() * movement;

	if (input::key('w'))
		_cam.transformation.position -= _cam.forward() * movement;

	if (input::key('s'))
		_cam.transformation.position += _cam.forward() * movement;

	if (input::key_up(' '))  // space
	{
		fps_mode = !fps_mode;
		if (fps_mode)
		{
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(800/2, 600/2);
		}
		else
			glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void game::_input(input_type type, unsigned char key, int x, int y)
{
	float const angular_movement = 0.1f;

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

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL triangle");

	glutKeyboardFunc(input::keyboard);
	glutKeyboardUpFunc(input::keyboard_up);
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
	while (true)
	{
		glutMainLoopEvent();
		input();

		update();
		input::update();

		render();
		glutSwapBuffers();
	}
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

void game::idle()
{}

void game::motion(int x, int y)
{
	game::ref()._input(input_type::motion, '\0', x, y);
}

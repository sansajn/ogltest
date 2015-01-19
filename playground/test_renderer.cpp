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
#include "texture.hpp"
#include "input.hpp"
#include "engine.hpp"
#include "component.hpp"

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
	void update(float dt) {_scene.update(dt);}
	void render();

	void _input(input_type type, unsigned char key, int x, int y);

	static void display();
	static void motion(int x, int y);
	static void idle();

private:
	game() : _cam(nullptr) {}
	void init();

	camera * _cam;
	renderer _rend;
	game_scene _scene;

	static bool fps_mode;
};

bool game::fps_mode = false;


void game::init()
{
	game_object * camera_obj = new game_object();
	_cam = new camera(70.0f, 800.0f/600.0f, 0.01f, 1000.0f);
	camera_obj->append_component(_cam);
	camera_obj->transformation.position = glm::vec3(0, 0, 5);
	_scene.append_object(camera_obj);
	_scene.camera_object(camera_obj);
	_scene.camera_to_screen(_cam->projection());

	ptr<mesh> plane(new mesh("plane.obj"));
	ptr<mesh> monkey(new mesh("monkey3.obj"));
	ptr<texture> bricks(new texture("bricks.jpg"));
	ptr<texture> checker(new texture("checker.png"));

	game_object * plane_obj = new game_object();
	plane_obj->append_component(new mesh_renderer(plane, make_ptr<material>(checker)));
	plane_obj->transformation.uniform_scale(10);
	plane_obj->transformation.position = glm::vec3(0, -2, 0);
	_scene.append_object(plane_obj);

	game_object * monkey_obj = new game_object();
	monkey_obj->append_component(new mesh_renderer(monkey, make_ptr<material>(bricks)));
	_scene.append_object(monkey_obj);
}

void game::render()
{
	shader::program * prog = &_rend.active_program();
	prog->use();

	// light
	ptr<shader::uniform> eye_position = prog->uniform_variable("eye_pos");
	*eye_position = _scene.camera_object()->transformation.position;

	ptr<shader::uniform> light_direction = prog->uniform_variable("light_direction");
	*light_direction = glm::normalize(glm::vec3(-1, -1, -1));

	ptr<shader::uniform> light_color = prog->uniform_variable("light_color");
	*light_color = glm::vec3(1,1,1);

	prog->uniform_variable("light_intensity", 0.4f);

	_rend.render(_scene);
}

void game::input()
{
	float const movement = 0.1f;

	if (input::key('a'))
		_cam->transformation().position -= _cam->right() * movement;

	if (input::key('d'))
		_cam->transformation().position += _cam->right() * movement;

	if (input::key('w'))
		_cam->transformation().position -= _cam->forward() * movement;

	if (input::key('s'))
		_cam->transformation().position += _cam->forward() * movement;

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
			_cam->transformation().rotate(glm::vec3(0,1,0), -angle);
		}

		if (dy != 0)
		{
			float angle = angular_movement * dy;
			_cam->transformation().rotate(_cam->right(), -angle);
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

	input::glut_bind();
	glutPassiveMotionFunc(game::motion);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
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
	unsigned t_last = glutGet(GLUT_ELAPSED_TIME);

	while (true)
	{
		unsigned t = glutGet(GLUT_ELAPSED_TIME);
		float dt = float(t_last - t)/1000.0f;
		t_last = t;

		glutMainLoopEvent();
		input();

		update(dt);
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

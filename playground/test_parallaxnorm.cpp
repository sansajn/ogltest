// testuje mapovanie normal
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
	void update(float dt) {_engine.update(dt);}
	void render() {_engine.render();}

	void _input(input_type type, unsigned char key, int x, int y);

	static void motion(int x, int y);
	static void idle() {}
	static void reshape(int w, int h);
	static void mouse(int button, int state, int x, int y);

private:
	game() : _cam(nullptr) {}
	void init();

	camera * _cam;
	engine _engine;

	static bool fps_mode;
};

bool game::fps_mode = false;


void game::init()
{
	game_object * camera_obj = new game_object();
	_cam = new camera(70.0f, 800.0f/600.0f, 0.01f, 1000.0f);
	camera_obj->append_component(_cam);
	camera_obj->transformation.position = glm::vec3(0, 0, 3);
	_engine.append(camera_obj);

	ptr<mesh> plane(new mesh("plane.obj"));
	ptr<mesh> monkey(new mesh("monkey3.obj"));
	ptr<texture> bricks(new texture("bricks.jpg"));
	ptr<texture> bricks_n(new texture("bricks_n.jpg"));
	ptr<texture> bricks_h(new texture("bricks_h.jpg"));
	ptr<texture> checker(new texture("checker.png"));

	game_object * plane_obj = new game_object();
	plane_obj->append_component(new mesh_renderer(plane, make_ptr<material>(checker)));
	plane_obj->transformation.uniform_scale(10);
	plane_obj->transformation.position = glm::vec3(0, -2, 0);
	_engine.append(plane_obj);

	game_object * monkey_obj = new game_object();
	monkey_obj->append_component(new mesh_renderer(monkey, make_ptr<material>(bricks)));
	_engine.append(monkey_obj);

	game_object * smallplane = new game_object(glm::vec3(2, 0, 1));
	smallplane->append_component(new mesh_renderer(plane, make_ptr<material>(bricks)));

	game_object * smallplane_n = new game_object(glm::vec3(0, 0, -2.2));
	smallplane_n->append_component(new mesh_renderer(plane, make_ptr<material>(bricks, bricks_n)));
	smallplane->append_child(smallplane_n);

	game_object * smallplane_h = new game_object(glm::vec3(0, 0, -2.2));
	smallplane_h->append_component(new mesh_renderer(plane, make_ptr<material>(bricks, bricks_n, bricks_h)));
	smallplane_n->append_child(smallplane_h);

	_engine.append(smallplane);

	game_object * dirlight_obj = new game_object();
	dirlight_obj->append_component(new directional_light);
	_engine.append(dirlight_obj);
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

	if (input::key('z'))
		_cam->transformation().position += glm::vec3(0, -1, 0) * movement;

	if (input::key('x'))
		_cam->transformation().position += glm::vec3(0, 1, 0) * movement;

	if (input::key(27))  // escape
	{
		fps_mode = false;
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

void game::mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		fps_mode = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutWarpPointer(800/2, 600/2);
	}
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL playground demo");

	input::glut_bind();  // only keyboard
	glutPassiveMotionFunc(game::motion);
	glutMouseFunc(game::mouse);
	glutReshapeFunc(game::reshape);

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

void game::motion(int x, int y)
{
	game::ref()._input(input_type::motion, '\0', x, y);
}

void game::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

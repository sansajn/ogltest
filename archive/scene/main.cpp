#include <string>
#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "program.h"
#include "cube.h"
#include "camera.h"
#include "scene.h"

using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

inline float to_deg(float rad) {return (180.0/M_PI)*rad;}
inline float to_rad(float deg) {return (M_PI/180.0)*deg;}

void init_glew();
void dump_hw_info();

int WIDTH = 800, HEIGHT = 600;

class camera_scene : public gl::base_scene
{
public:
	camera_scene();
	bool init();
	void render();
	void close();
	void keyboard(unsigned char key, int x, int y);  // user input

private:
	gl::camera _cam;
	gl::program _prog;
};

camera_scene::camera_scene()
{}

bool camera_scene::init()
{
	_prog << "shader/color.vs" << "shader/color.fs";
	_prog.link();
	_prog.use();
	_cam.reset(glm::vec3(5, 5, 5), glm::vec3(.5f, 0, 0));
	return true;
}

void camera_scene::close()
{
	_prog.unuse();
}

void camera_scene::render()
{
	GL_CHECK_ERRORS;

	glm::mat4 project = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f);
	glm::mat4 vp = project * _cam.view();

	gl::vbo_cube c1;
	gl::vbo_cube c2;
	gl::vbo_cube c3;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 model(1.0f);
	glm::mat4 mvp = vp*model;
	_prog.uniform("mvp", mvp);
	_prog.uniform("model_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	c1.render();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.25f, 0.0f, 0.25f));
	mvp = vp*model;
	_prog.uniform("mvp", mvp);
	_prog.uniform("model_color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	c2.render();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 0.0f, -1.25f));
	mvp = vp*model;
	_prog.uniform("mvp", mvp);
	_prog.uniform("model_color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	c3.render();

	GL_CHECK_ERRORS;
}

void camera_scene::keyboard(unsigned char key, int x, int y)
{
	float dt = .5f;

	switch (key)
	{
		case 'a':
			_cam.left(dt);
			break;

		case 'd':
			_cam.right(dt);
			break;

		case 'w':
			_cam.up(dt);
			break;

		case 's':
			_cam.down(dt);
			break;
	};

	_cam.update();
	glm::vec3 rot = _cam.rotation();
	cout << "yaw:" << to_deg(rot.x) << ", pitch:" << to_deg(rot.y) << "\n";
}

camera_scene scene;


void on_render()
{
	scene.render();
	glutSwapBuffers();
	glutPostRedisplay();
}

void on_keyboard(unsigned char key, int x, int y)
{
	scene.keyboard(key, x, y);
	glutPostRedisplay();
}

void init()
{
	init_glew();

	glEnable(GL_DEPTH_TEST);

	scene.init();
}

void on_close()
{
	scene.close();
}


int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("vbo cube test");

	init();
	dump_hw_info();

	glutDisplayFunc(on_render);
	glutCloseFunc(on_close);
	glutKeyboardFunc(on_keyboard);

	glutMainLoop();

	return 0;
}

void init_glew()
{
	glewExperimental = GL_TRUE;  // inak vo verzii 1.8 padne na glGenVertexArrays()
	GLenum state = glewInit();
	if (state != GLEW_OK)
		cerr << "Error: " << glewGetErrorString(state) << endl;

	if (GLEW_VERSION_4_0)
		cout << "OpenGL 4.0 supported\n";

	glGetError();  // swallow error 1282
	GL_CHECK_ERRORS;
}

void dump_hw_info()
{
	cout << "\tUsing GLEW "<< glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

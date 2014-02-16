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

gl::program prog;
gl::camera cam;
int WIDTH = 800, HEIGHT = 600;


void on_render()
{
	GL_CHECK_ERRORS;

	glm::mat4 project = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f);
	glm::mat4 vp = project * cam.view();

	gl::vbo_cube c1;
	gl::vbo_cube c2;
	gl::vbo_cube c3;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 model(1.0f);
	glm::mat4 mvp = vp*model;
	prog.uniform("mvp", mvp);
	prog.uniform("model_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	c1.render();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.25f, 0.0f, 0.25f));
	mvp = vp*model;
	prog.uniform("mvp", mvp);
	prog.uniform("model_color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	c2.render();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 0.0f, -1.25f));
	mvp = vp*model;
	prog.uniform("mvp", mvp);
	prog.uniform("model_color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	c3.render();

	GL_CHECK_ERRORS;

	glutSwapBuffers();

	glutPostRedisplay();
}

void on_keyboard(unsigned char key, int x, int y)
{
	float dt = .5f;

	switch (key)
	{
		case 'a':
			cam.left(dt);
			break;

		case 'd':
			cam.right(dt);
			break;

		case 'w':
			cam.up(dt);
			break;

		case 's':
			cam.down(dt);
			break;
	};

	cam.update();
	glm::vec3 rot = cam.rotation();
	cout << "yaw:" << to_deg(rot.x) << ", pitch:" << to_deg(rot.y) << "\n";

	glutPostRedisplay();
}

void init()
{
	init_glew();	

	glEnable(GL_DEPTH_TEST);
}

void on_close()
{
	prog.unuse();
}


int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("vbo cube test");

	init();

	dump_hw_info();

	prog << "shader/color.vs" << "shader/color.fs";
	prog.link();
	prog.use();

	cam.reset(glm::vec3(5, 5, 5), glm::vec3(.5f, 0, 0));

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

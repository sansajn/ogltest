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

using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

inline float to_deg(float rad) {return (180.0/M_PI)*rad;}
inline float to_rad(float deg) {return (M_PI/180.0)*deg;}

void dump_vec3(glm::vec3 const & v, string const & name);

void init_glew();


int WIDTH = 800, HEIGHT = 600;

gl::program prog;
float yaw = .0f, pitch = .0f;
glm::vec3 eye(5,0,5);


void on_render()
{
	GL_CHECK_ERRORS;

	glm::mat4 project = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.3f, 100.0f);

	glm::mat4 R = glm::yawPitchRoll(yaw, pitch, .0f);
	glm::vec3 look = glm::vec3(R*glm::vec4(0,0,-1,0));
	glm::vec3 center = eye + look;
	glm::vec3 up(0,1,0);
	glm::mat4 V = glm::lookAt(eye, center, up);

	dump_vec3(look, "look");

/*
	glm::mat4 V = glm::lookAt(glm::vec3(.0f, .0f, 5.0f),
		glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, 1.0f, .0f));
*/

	glm::mat4 vp = project * V;

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

	GL_CHECK_ERRORS;

	glutSwapBuffers();

//	glutPostRedisplay();
}

void on_keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'a':
			yaw += to_rad(5);
			break;

		case 'd':
			yaw -= to_rad(5);
			break;

		case 'w':
			pitch += to_rad(5);
			break;

		case 's':
			pitch -= to_rad(5);
			break;
	};

	cout << "yaw:" << to_deg(yaw) << ", pitch:" << to_deg(pitch) << "\n";

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

	prog << "shader/color.vs" << "shader/color.fs";
	prog.link();
	prog.use();

	yaw = atan2(eye.x, eye.z);
	pitch = -atan2(eye.y, sqrt(eye.x*eye.x + eye.z*eye.z));
	cout << "yaw:" << yaw << ", pitch:" << pitch << "\n";

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

void dump_vec3(glm::vec3 const & v, string const & name)
{
	cout << name << " = ["
		<< v[0] << " " << v[1] << " " << v[2] << "]\n";
}

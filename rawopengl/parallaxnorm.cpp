// implementacia normal mapping-u a prallax displacement mapping-u
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "camera.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"

unsigned width = 800;
unsigned height = 600;

void init(int argc, char * argv[]);
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void motion(int x, int y);

void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

mesh * plane = nullptr;
texture * bricks = nullptr;
texture * bricks_n = nullptr;
texture * bricks_h = nullptr;
shader::program * prog = nullptr;
camera * cam = nullptr;

bool fps_mode = false;


void display()
{
	glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(5, 5, 5));
	glm::mat4 V = cam->view();
	glm::mat4 P = cam->projection();
	glm::mat3 N = glm::mat3(glm::inverseTranspose(M*V));

	prog->use();
	prog->uniform_variable("camera_to_screen", P);
	prog->uniform_variable("world_to_camera", V);
	prog->uniform_variable("local_to_world", M);
	prog->uniform_variable("normal_to_camera", N);

	prog->uniform_variable("light.direction", glm::vec3(0,-1,-1));
	prog->uniform_variable("light.color", glm::vec3(1,1,1));
	prog->uniform_variable("light.intensity", 1.0f);

	prog->uniform_variable("material.ambient", glm::vec3(0.1, 0.1, 0.1));
	prog->uniform_variable("material.shininess", 64.0f);  // 1.0 - 128.0
	prog->uniform_variable("material.intensity", 0.4f);   // 0.0 - 1.0

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	bricks->bind(0);
	prog->uniform_variable("diffuse", 0);

	bricks_n->bind(1);
	prog->uniform_variable("normalmap", 1);

	bricks_h->bind(2);
	prog->uniform_variable("heightmap", 2);

	plane->draw();

	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char * argv[])
{
	init(argc, argv);
	glEnable(GL_DEPTH_TEST);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog = new shader::program("shaders/parallaxnorm.glsl");
	plane = new mesh("models/plane.obj");
	bricks = new texture("textures/bricks.png");
	bricks_n = new texture("textures/bricks_n.png");
	bricks_h = new texture("textures/bricks_h.png");
	cam = new camera(glm::vec3(0,1,0), 70, float(width)/float(height), 0.01, 1000);

	glutMainLoop();

	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL normal mapping demo");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

void motion(int x, int y)
{
	if (!fps_mode)
		return;

	unsigned center_w = width/2;
	unsigned center_h = height/2;
	float const angular_movement = 0.1f;

	int dx = x - center_w;
	int dy = y - center_h;

	if (dx != 0)
	{
		float angle = angular_movement*dx;
		cam->rotation = glm::normalize(glm::angleAxis(-angle, glm::vec3(0,1,0)) * cam->rotation);
	}

	if (dy != 0)
	{
		float angle = angular_movement*dy;
		cam->rotation = glm::normalize(glm::angleAxis(-angle, cam->right()) * cam->rotation);
	}

	if (dx != 0 || dy != 0)
		glutWarpPointer(center_w, center_h);
}

void keyboard(unsigned char key, int x, int y)
{
	float const linear_movement = 0.4f;

	if (key == 27)  // escape
	{
		fps_mode = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (key == 'a')
		cam->position -= linear_movement * cam->right();

	if (key == 'd')
		cam->position += linear_movement * cam->right();

	if (key == 'w')
		cam->position -= linear_movement * cam->forward();

	if (key == 's')
		cam->position += linear_movement * cam->forward();
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		fps_mode = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}
}

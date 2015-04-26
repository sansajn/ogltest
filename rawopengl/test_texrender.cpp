// renderovanie do textúry
#include <string>
#include <cassert>
#include <iostream>
#include <string>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <ImageMagick/Magick++.h>
#include "camera.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"

using std::string;

unsigned width = 800;
unsigned height = 600;

void init(int argc, char * argv[]);
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void motion(int x, int y);

void dump_texture(GLuint render_tex, int texw, int texh, string const & fname);

mesh * plane = nullptr;
texture2d * bricks = nullptr;
texture2d * bricks_n = nullptr;
texture2d * bricks_h = nullptr;
shader::program * prog = nullptr;
camera * cam = nullptr;

GLuint render_tex = 0;
GLuint depth_rbo = 0;
GLuint render_fbo = 0;
int texw = 400, texh = 300;

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

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	prog = new shader::program("assets/shaders/parallaxnorm.glsl");
	plane = new mesh("assets/models/plane.obj");
	bricks = new texture2d("assets/textures/bricks.png");
	bricks_n = new texture2d("assets/textures/bricks_n.png");
	bricks_h = new texture2d("assets/textures/bricks_h.png");
	cam = new camera(glm::vec3(0,1,0), glm::radians(70.0f), float(width)/float(height), 0.01, 1000);

	// generate a texture to render into
	glGenTextures(1, &render_tex);
	glBindTexture(GL_TEXTURE_2D, render_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texw, texh, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// create a depth buffer renderbuffer
	glGenRenderbuffers(1, &depth_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, texw, texh);

	// create a framebuffer and attach a texture and a depth buffer to it
	glGenFramebuffers(1, &render_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(false && "framebuffer is not complete");

	glEnable(GL_DEPTH_TEST);

	// use framebuffer instead of default window
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_fbo);
	glViewport(0, 0, texw, texh);
	glClearColor(0, 0, 0, 1);

	display();

	dump_texture(render_tex, texw, texh, "out/fbo_texture.png");

	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL normal and parallax displacement mapping demo");
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
		float angle = glm::radians(angular_movement*dx);
		cam->rotation = glm::normalize(glm::angleAxis(-angle, glm::vec3(0,1,0)) * cam->rotation);
	}

	if (dy != 0)
	{
		float angle = glm::radians(angular_movement*dy);
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

void dump_texture(GLuint tex, int w, int h, string const & fname)
{
	std::unique_ptr<uint8_t []> buff(new uint8_t[w*h*4]);
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff.get());

	// TODO: cez ten blob sa to neda ?

	Magick::Image im;
	im.read(w, h, "RGBA", Magick::StorageType::CharPixel, buff.get());
	im.flip();
	im.write(fname);
}

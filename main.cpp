#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "program.h"

using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


class vbo_cube
{
public:
	vbo_cube();
	void render();

private:
	GLuint _vao;
};

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

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

int main(int argc, char * argv[])
{
	int w = 800, h = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(w, h);
	glutCreateWindow("vbo cube test");

	init_glew();

	dump_hw_info();

	gl::program prog;
//	prog.compile("simple.vs", GL_VERTEX_SHADER);
//	prog.compile("simple.fs", GL_FRAGMENT_SHADER);
	prog.compile("simple.vs");
	prog.compile("simple.fs");
	prog.link();
	prog.use();

	GL_CHECK_ERRORS;

	cout << "program compiled, linked and used\n";

	glm::mat4 project = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.25f, 1.25f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model = glm::mat4(1.0f);

	glm::mat4 mvp = project*view*model;

	cout << "matrices created\n";

	prog.uniform("mvp", mvp);

	vbo_cube cube;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	cube.render();

	GL_CHECK_ERRORS;

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}


vbo_cube::vbo_cube()
	: _vao(0)
{
	float side = 1.0f;
	float side2 = side/2.0f;

	float v[24*3] = {
		// Front
		-side2, -side2, side2,
		 side2, -side2, side2,
		 side2,  side2, side2,
		-side2,  side2, side2,
		// Right
		 side2, -side2, side2,
		 side2, -side2, -side2,
		 side2,  side2, -side2,
		 side2,  side2, side2,
		// Back
		-side2, -side2, -side2,
		-side2,  side2, -side2,
		 side2,  side2, -side2,
		 side2, -side2, -side2,
		// Left
		-side2, -side2, side2,
		-side2,  side2, side2,
		-side2,  side2, -side2,
		-side2, -side2, -side2,
		// Bottom
		-side2, -side2, side2,
		-side2, -side2, -side2,
		 side2, -side2, -side2,
		 side2, -side2, side2,
		// Top
		-side2,  side2, side2,
		 side2,  side2, side2,
		 side2,  side2, -side2,
		-side2,  side2, -side2
	};

	float n[24*3] = {
		// Front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		// Back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		// Left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		// Bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		// Top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint el[] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	glGenVertexArrays(1, &_vao);  // vytvor 1 vertex-array
	glBindVertexArray(_vao);

	unsigned int bufs[3];
	glGenBuffers(3, bufs);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), n, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), el, 
		GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_cube::render()
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

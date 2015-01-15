// nakresí trojuholník iba za použitia gl-funkcií (bez transformácie)
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
#include "program.hpp"
#include "utils.hpp"
#include "mesh.hpp"

using std::string;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * vs_src = "#version 330\n\
							  layout(location=0) in vec3 position;\n\
							  layout(location=1) in vec4 color;\n\
							  out VS_OUT {\n\
								  vec4 color;\n\
							  } vs_out;\n\
							  void main() {\n\
								  vs_out.color = color;\n\
								  gl_Position = vec4(position, 1.0f);\n\
							  }\n";

char const * fs_src = "#version 330\n\
							  in VS_OUT {\n\
								  vec4 color;\n\
							  } fs_in;\n\
							  out vec4 fcolor;\n\
							  void main() {\n\
								  fcolor = fs_in.color;\n\
							  }\n";

void init(int argc, char * argv[]);


void dump_compile_log(GLuint shader, std::string const & name)
{
	GLint len;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	std::string log;
	log.resize(len);
	glGetShaderInfoLog(shader, len, nullptr, (GLchar *)log.data());
	std::cout << "compile output ('" << name << "'):\n" << log << std::endl;
}

void dump_link_log(GLuint program, std::string const & name)
{
	GLint len;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
	std::string log;
	log.resize(len);
	glGetProgramInfoLog(program, len, nullptr, (GLchar *)log.data());
	std::cout << "link output ('" << name << "'):\n" << log << std::endl;
}

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	string shader_code = read_file("simple.glsl");
	shader::program prog(make_ptr<shader::module>(shader_code));

	prog.use();

	ptr<shader::uniform> MVP = prog.uniform_variable("MVP");
	*MVP = glm::mat4(glm::vec4(0.5, 0, 0, 0), glm::vec4(0, 0.5, 0, 0), glm::vec4(0, 0, 0.5, 0), glm::vec4(0, 0, 0, 1));

	mesh m("plane_xy.obj");

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	m.draw();
	glutSwapBuffers();

	glutMainLoop();

	return 0;
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

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

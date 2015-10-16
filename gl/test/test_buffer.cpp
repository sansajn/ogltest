// test gpu_buffer-u
#include <string>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "mesh.hpp"

using gl::gpu_buffer;
using gl::buffer_usage;
using gl::buffer_target;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * vs_src = R"(
	#version 330
	layout(location=0) in vec3 position;
	layout(location=1) in vec4 color;
	out VS_OUT {
	  vec4 color;
	} vs_out;
	void main() {
	  vs_out.color = color;
	  gl_Position = vec4(position, 1.0f);
	}
)";

char const * fs_src = R"(
	#version 330
	in VS_OUT {
	  vec4 color;
	} fs_in;
	out vec4 fcolor;
	void main() {
	  fcolor = fs_in.color;
	}
)";

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

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, nullptr);
	glCompileShader(vs);

	// check for compile errors
	GLint compiled;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(vs, "vertex-shader");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, nullptr);
	glCompileShader(fs);

	// check for compile errors ...
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(fs, "fragment-shader");

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);	

	// check for link errors ...
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(prog, "vertex-shader;fragment-shader");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat vbuf_data[] = {  // position:3, color:4
		-.5f, -.5f, .0f, 1.0f,  .0f, .0f, 1.0f,
		 .5f, -.5f, .0f,  .0f, 1.0f, .0f, 1.0f,
		 .0f,  .5f, .0f,  .0f, .0f, 1.0f, 1.0f};

	unsigned indices[] = {0, 1, 2};

	gpu_buffer vbuf{vbuf_data, sizeof(vbuf_data), buffer_usage::static_draw};
	gpu_buffer ibuf{indices, sizeof(indices), buffer_usage::static_draw};

	vbuf.bind(buffer_target::array);
	ibuf.bind(buffer_target::element_array);

	GLuint position_attr_id = 0, color_attr_id = 1;
	glVertexAttribPointer(position_attr_id, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), 0);
	glEnableVertexAttribArray(position_attr_id);

	glVertexAttribPointer(color_attr_id, 4, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), BUFFER_OFFSET(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(color_attr_id);

	// rendering ...
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(prog);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

	glutSwapBuffers();

	glutMainLoop();

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);

	return 0;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL gpu buffer test");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

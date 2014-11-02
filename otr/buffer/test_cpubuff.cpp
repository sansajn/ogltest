// test gpu-buffera stride rozloženie
#include <string>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render/cpubuffer.hpp"
#include "render/gpubuffer.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * fs_src = "#version 330\n\
							  uniform sampler2D sampler;\n\
							  uniform vec2 scale;\n\
							  out vec4 fcolor;\n\
							  void main() {\n\
								  fcolor = texture(sampler, gl_FragCoord.xy * scale).rrrr;\n\
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

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, nullptr);
	glCompileShader(fs);

	// check for compile errors ...
	GLint compiled;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(fs, "fragment-shader");

	GLuint prog = glCreateProgram();
	glAttachShader(prog, fs);
	glLinkProgram(prog);	

	// check for link errors ...
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(prog, "vertex-shader;fragment-shader");

	GLfloat vertices[] = {
		// t1
		 1,  1,
		-1,  1,
		-1, -1,
		// t2
		 1,  1,
		-1, -1,
		 1, -1
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	gpubuffer buf;
	buf.data(6*2*sizeof(GLfloat), vertices, buffer_usage::STATIC_DRAW);
	buf.bind(GL_ARRAY_BUFFER);

	GLuint position_attr_id = 0;
	glVertexAttribPointer(position_attr_id, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attr_id);

	glActiveTexture(GL_TEXTURE0);  // aktyvuj texturovaciu jednotku 0
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	unsigned char data[16] = {
		0, 255, 0, 255,
		255, 0, 255, 0,
		0, 255, 0, 255,
		255, 0, 255, 0};

	cpubuffer texbuf(data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid *)texbuf.data(0));

	GLint sampler_loc = glGetUniformLocation(prog, "sampler");
	GLint scale_loc = glGetUniformLocation(prog, "scale");

	// rendering ...
	glUseProgram(prog);

	glm::vec2 scaler(1.0f/800.0f, 1.0f/600.0f);

	glUniform1i(sampler_loc, 0);  // 0 - texture unit
	glUniform2fv(scale_loc, 1, glm::value_ptr(scaler));

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glutSwapBuffers();

	glutMainLoop();

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
	glutCreateWindow("OpenGL triangle");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

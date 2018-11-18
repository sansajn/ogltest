// gl- shader and program sample
#include <iostream>
#include <GLFW/glfw3.h>
#include "gl/glfw3_window.hpp"

using std::cout;

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;
static const GLchar* vertex_shader_source =
		"#version 100\n"
		"attribute vec3 position;\n"
		"void main() {\n"
		"   gl_Position = vec4(position, 1.0);\n"
		"}\n";
static const GLchar* fragment_shader_source =
		"#version 100\n"
		"void main() {\n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n";
static const GLfloat vertices[] = {
	0.0f,  0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
};

GLint common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source);


class scene : public ui::glfw_window
{
public:
	using base = ui::glfw_window;

	scene()
	{
		cout << "GL_VERSION  : " << glGetString(GL_VERSION) << "\n";
		cout << "GL_RENDERER : " << glGetString(GL_RENDERER) << "\n";

		_shader_program = common_get_shader_program(vertex_shader_source, fragment_shader_source);
		_pos = glGetAttribLocation(_shader_program, "position");

		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(_pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(_pos);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	~scene() override
	{
		glDeleteBuffers(1, &_vbo);
	}

	void display() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(_shader_program);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		base::display();
	}

private:
	GLuint _shader_program, _vbo;
	GLint _pos;
};



int main(int argc, char * argv[])
{
	scene s;
	s.start();

	return EXIT_SUCCESS;
}

GLint common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) {
	enum Consts {INFOLOG_LEN = 512};
	GLchar infoLog[INFOLOG_LEN];
	GLint fragment_shader;
	GLint shader_program;
	GLint success;
	GLint vertex_shader;

	/* Vertex shader */
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
	}

	/* Fragment shader */
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
	}

	/* Link shaders */
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	return shader_program;
}

#include "module.hpp"
#include <sstream>
#include <utility>
#include <exception>
#include <iostream>
#include <GL/glew.h>

namespace shader {

void module::init(int version, char const * source)
{
	init(version,
		"#define _VERTEX_\n", strstr(source, "_VERTEX_") ? source : nullptr,
		"#define _FRAGMENT_\n", strstr(source, "_FRAGMENT_") ? source : nullptr);
}

void module::init(int version, char const * vertex, char const * fragment)
{
	init(version, nullptr, vertex, nullptr, fragment);
}

void module::init(int version, char const * vertex_header, char const * vertex, char const * fragment_header, char const * fragment)
{
	std::ostringstream oss;
	oss << "#version " << version << "\n";
	std::string version_line = oss.str();

	char const * lines[3];
	lines[0] = version_line.c_str();

	bool error = false;

	if (vertex)  // create vertex shader object
	{
		int nlines = vertex_header ? 3 : 2;
		lines[1] = vertex_header ? vertex_header : vertex;
		lines[2] = vertex;
		_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(_vertex_shader_id, nlines, lines, NULL);
		glCompileShader(_vertex_shader_id);
		error = !check(_vertex_shader_id);
		print_log(_vertex_shader_id, nlines, lines, error);
		if (error)
		{
			glDeleteShader(_vertex_shader_id);
			_vertex_shader_id = -1;
			throw std::exception();
		}
	}
	else
		_vertex_shader_id = -1;

	if (fragment)  // create fragmeent shader object
	{
		int nlines = fragment_header ? 3 : 2;
		lines[1] = fragment_header ? fragment_header : fragment;
		lines[2] = fragment;
		_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(_fragment_shader_id, nlines, lines, NULL);
		glCompileShader(_fragment_shader_id);
		error = !check(_fragment_shader_id);
		print_log(_fragment_shader_id, nlines, lines, error);
		if (error)
		{
			if (_vertex_shader_id != -1)
			{
				glDeleteShader(_vertex_shader_id);
				_vertex_shader_id = -1;
			}

			glDeleteShader(_fragment_shader_id);
			_fragment_shader_id = -1;
			throw std::exception();
		}
	}
	else
		_fragment_shader_id = -1;
}

module::~module()
{
	if (_vertex_shader_id != -1)
		glDeleteShader(_vertex_shader_id);

	if (_fragment_shader_id != -1)
		glDeleteShader(_fragment_shader_id);

	_users.clear();
	_initial_values.clear();
}

void module::initial_value(ptr<any_value> value)
{
	// TODO: subroutine not supported
	_initial_values.insert(std::make_pair(value->name(), value));
}

bool module::check(int shader_id)
{
	GLint compiled;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
	return compiled != 0;
}

void module::print_log(int shader_id, int nlines, char const ** lines, bool error)
{
	GLint loglen;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &loglen);

	char * log = new char[loglen];
	GLsizei len;
	glGetShaderInfoLog(shader_id, GLsizei(loglen), &len, log);

	std::cout << log << std::endl;

	delete [] log;
}

}  // shader

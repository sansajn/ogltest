#include "program.hpp"
#include <iostream>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

using std::string;

namespace shader {

void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

program::program(ptr<module> m)
{
	_pid = glCreateProgram();

	unsigned const * ids = m->ids();
	for (int i = 0; i < 2; ++i)
		glAttachShader(_pid, ids[i]);  // TODO: iteruj cez rozsah

	glLinkProgram(_pid);

	bool result = link_check();
	assert(result && "program linking failed");
	if (!result)
		throw std::exception();  // TODO: specify exception

	_modules.push_back(m);
}

bool program::link_check()
{
	GLint linked;
	glGetProgramiv(_pid, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(_pid, "<unspecified>");
	return linked == GL_FALSE;
}

module::module(string code)
{
	_ids[0] = _ids[1] = 0;

	if (code.find("_VERTEX_") != string::npos)
		compile(code, shader_type::vertex);

	if (code.find("_FRAGMENT_") != string::npos)
		compile(code, shader_type::fragment);
}

void module::compile(std::string code, shader_type type)
{
	char const * lines[3];
	lines[0] = "#version 330";

	unsigned sid;
	GLenum stype;

	switch (type)
	{
		case shader_type::vertex:
			sid = 0;
			lines[1] = 	"#define _VERTEX_";
			stype = GL_VERTEX_SHADER;
			break;

		case shader_type::fragment:
			sid = 1;
			lines[1] = "#define _FRAGMENT_";
			stype = GL_FRAGMENT_SHADER;
			break;

		default:
			assert(0 && "unsupported shader program type");
	}

	lines[2] = code.c_str();

	_ids[sid] = glCreateShader(stype);
	glShaderSource(_ids[sid], 3, lines, nullptr);
	glCompileShader(_ids[sid]);

	bool result = compile_check(_ids[sid], type);
	assert(result && "shader compile failed");
	if (!result)
		throw std::exception();  // TODO: specify exception
}

bool module::compile_check(unsigned sid, shader_type type)
{
	string name;
	switch (type)
	{
		case shader_type::vertex:
			name = "vertex-shader";
			break;

		case shader_type::fragment:
			name = "fragment-shader";
			break;

		default:
			assert(0 && "unsupported shader program type");
	}

	GLint compiled;
	glGetShaderiv(sid, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(sid, name);

	return compiled == GL_FALSE;
}

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

template <>
void set_uniform<glm::mat3>(int location, glm::mat3 const & v)
{
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(v));
}

template <>
void set_uniform<glm::mat4>(int location, glm::mat4 const & v)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(v));
}

template <>
void set_uniform<glm::vec2>(int location, glm::vec2 const & v)
{
	glUniform2fv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<glm::vec3>(int location, glm::vec3 const & v)
{
	glUniform3fv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<glm::vec4>(int location, glm::vec4 const & v)
{
	glUniform4fv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<glm::ivec2>(int location, glm::ivec2 const & v)
{
	glUniform2iv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<glm::ivec3>(int location, glm::ivec3 const & v)
{
	glUniform3iv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<glm::ivec4>(int location, glm::ivec4 const & v)
{
	glUniform4iv(location, 1, glm::value_ptr(v));
}

template <>
void set_uniform<int>(int location, int const & v)
{
	glUniform1i(location, v);
}

template<>
void set_uniform<float>(int location, float const & v)
{
	glUniform1f(location, v);
}

template <>
void get_uniform(unsigned program, int location, int & v)
{
	glGetUniformiv(program, location, &v);
}

}  // shader

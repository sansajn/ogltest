#include "program.h"
#include <map>
#include <memory>
#include <sstream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>

//debug
#include <iostream>

namespace gl {

using std::string;
using std::unique_ptr;
using std::stringstream;
using std::ifstream;
using std::map;


string shader_info_log(GLuint shader);
string program_info_log(GLuint program);


namespace detail {

namespace fs = boost::filesystem;

class shader_type
{
public:
	struct shader_type_desc
	{
		GLenum type;
		char const * ext_list;
	};

	shader_type(shader_type_desc * desc)
	{
		typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

		while (desc->ext_list)
		{
			string s = desc->ext_list;
			for (string ext : tokenizer(s, boost::char_separator<char>(";")))
				_types[ext] = desc->type;
			++desc;
		}
	}

	GLenum deduce(char const * filename)
	{
		auto it = _types.find(fs::extension(filename));
		if (it != _types.end())
			return it->second;
		else
			throw program_exception(boost::str(
				boost::format("unknown shader type '%1%'") % fs::extension(filename)));
	}

private:
	map<string, GLuint> _types;
};

}  // detail

detail::shader_type::shader_type_desc shader_desc[]
{
	{GL_VERTEX_SHADER, ".vs;.vert"},
	{GL_FRAGMENT_SHADER, ".fs;.frag"},
	{GL_GEOMETRY_SHADER, ".gs;.geom"},
	{0, 0}
};

detail::shader_type shader_type_info(shader_desc);


program::program()
	: _program(0), _linked(false)
{}

program::~program()
{
	if (_program == 0)
		return;

	GLint nshaders = 0;
	glGetProgramiv(_program, GL_ATTACHED_SHADERS, &nshaders);

	unique_ptr<GLuint[]> shaders(new GLuint[nshaders]);
	glGetAttachedShaders(_program, nshaders, NULL, shaders.get());

	for (int i = 0; i < nshaders; ++i)
		glDeleteShader(shaders[i]);

	glDeleteProgram(_program);
}

void program::compile(char const * filename)
{
	compile(filename, shader_type_info.deduce(filename));
}

void program::compile(char const * filename, GLenum type)
{
	create_program_lazy();

	string source = read_shader(filename);

	GLuint shader = glCreateShader(type);
	char const * src = source.c_str();
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	// error handling
	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		string log = shader_info_log(shader);
		throw program_exception(boost::str(boost::format(
			"can't compile '%1%' shader, reason: %2%") % filename % log));
	}

	glAttachShader(_program, shader);
}

void program::link()
{
	if (_linked)
		return;

	if (_program < 1)
		throw program_exception("program has not been compiled");

	glLinkProgram(_program);

	// error handling
	GLint status;
	glGetProgramiv(_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		string log = program_info_log(_program);
		throw program_exception("program link failed, reason:" + log);
	}

	_linked = true;
}

void program::use() const
{
	if (!_linked)
		throw program_exception("program has not been linked");

	glUseProgram(_program);
}

void program::unuse() const
{
	glUseProgram(0);
}

GLint program::uniform_location(char const * name)
{
	auto it = _uniforms.find(name);
	if (it == _uniforms.end())
		_uniforms[name] = glGetUniformLocation(_program, name);
	return _uniforms[name];
}

void program::create_program_lazy()
{
	if (_program < 1)
		_program = glCreateProgram();

	if (_program < 1)
		throw program_exception("unable to create shader program");
}

std::string program::read_shader(char const * filename)
{
	ifstream in(filename);
	stringstream ss;
	ss << in.rdbuf();
	in.close();
	return ss.str();
}

string shader_info_log(GLuint shader)
{
	GLint length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		unique_ptr<char[]> buf(new char[length]);
		GLint written = 0;
		glGetShaderInfoLog(shader, length, &written, buf.get());
		return string(buf.get());
	}
	else
		return "";
}

string program_info_log(GLuint program)
{
	GLint length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		unique_ptr<char[]> buf(new char[length]);
		GLint written;
		glGetProgramInfoLog(program, length, &written, buf.get());
		return string(buf.get());
	}
	else
		return "";
}

template <>
void uniform_upload<glm::mat3>(GLuint location, glm::mat3 const & m)
{
	glUniformMatrix3fv(location, 1, GL_FALSE, &m[0][0]);
}

template <>
void uniform_upload<glm::mat4>(GLuint location, glm::mat4 const & m)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
}

template <>
void uniform_upload<glm::vec3>(GLuint location, glm::vec3 const & v)
{
	glUniform3fv(location, 1, (float *)&v);
}

template <>
void uniform_upload<glm::vec4>(GLuint location, glm::vec4 const & v)
{
	glUniform4fv(location, 1, (float *)&v);
}

};  // gl

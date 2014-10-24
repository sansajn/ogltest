#include "program.h"
#include <map>
#include <memory>
#include <string>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/utils.hpp"

using std::map;
using std::unique_ptr;
using std::string;
using std::stringstream;
using std::ifstream;

static string shader_info_log(GLuint shader);
static string program_info_log(GLuint shader_program);

namespace detail {

namespace fs = boost::filesystem;

struct shader_desc
{
	GLenum type;
	char const * ext_list;
};

shader_desc shader_desc_table[] =
{
	{GL_VERTEX_SHADER, ".vs;.vert"},
	{GL_FRAGMENT_SHADER, ".fs;.frag"},
	{GL_GEOMETRY_SHADER, ".gs;.geom"},
	{0, 0}
};

class shader_info
{
public:
	static shader_info & ref();
	GLenum type(char const * filename);

private:
	shader_info(shader_desc * desc);

	std::map<std::string, GLuint> _types;
};

shader_info & shader_info::ref()
{
	static shader_info info(shader_desc_table);
	return info;
}

shader_info::shader_info(shader_desc * desc)
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

GLenum shader_info::type(char const * filename)
{
	auto it = _types.find(fs::extension(filename));
	if (it != _types.end())
		return it->second;
	else
		throw shader_program_exception(boost::str(
			boost::format("unknown shader type '%1%'") % fs::extension(filename)));
}

bool program_used(GLuint program_id)
{
	GLint id = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);
	return id == program_id;
}

}  // detail

uniform_variable::uniform_variable(char const * name, shader_program const & prog)
{
	link(name, prog);
}

void uniform_variable::link(const char * name, shader_program const & prog)
{
	if (!prog.used())
		throw shader_program_exception("accessing uniform in unused program (call use() before)");

	_location = glGetUniformLocation(prog.id(), name);
	if (_location == -1)
		throw shader_program_exception(boost::str(boost::format(
			"'%1%' does not correspond to an active uniform variable") % name));
}

shader_module::shader_module(char const * fname)
{
	compile(fname, detail::shader_info::ref().type(fname));
}

shader_module::shader_module(char const * fname, GLenum type)
{
	compile(fname, type);
}

shader_module::~shader_module()
{
	glDeleteShader(_id);
}

void shader_module::compile(char const * fname, GLenum type)
{
	_id = glCreateShader(type);
	_type = type;

	string source(read_file(fname));
	char const * src = source.c_str();
	glShaderSource(_id, 1, &src, NULL);
	glCompileShader(_id);

	// error handling
	int result;
	glGetShaderiv(_id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		string log = shader_info_log(_id);
		throw shader_program_exception(boost::str(boost::format(  // TODO: zmen na shader_exception
			"can't compile '%1%' shader, reason: %2%") % fname % log));
	}
}

shader_program const * shader_program::_CURRENT = nullptr;

shader_program::shader_program()
	: _id(0), _linked(false)
{}

shader_program::~shader_program()
{
	if (_id == 0)
		return;

	if (used())
		unuse();

	glDeleteProgram(_id);
}

void shader_program::attach(ptr<shader_module> module)
{
	create_program_lazy();
	_modules.push_back(module);
	glAttachShader(_id, module->id());
}

void shader_program::attach(char const * fname)
{
	ptr<shader_module> m = make_ptr<shader_module>(fname);
	attach(m);
}

void shader_program::attach(char const * fname, GLenum type)
{
	ptr<shader_module> m = make_ptr<shader_module>(fname, type);
	attach(m);
}

void shader_program::link()
{
	if (_linked)
		return;

	if (_id < 1)
		throw shader_program_exception("program has not been compiled");

	glLinkProgram(_id);

	// error handling
	GLint status;
	glGetProgramiv(_id, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		string log = program_info_log(_id);
		throw shader_program_exception("program link failed, reason:" + log);
	}

	_linked = true;
}

void shader_program::use() const
{
	if (!_linked)
		throw shader_program_exception("program has not been linked");

	glUseProgram(_id);
	_CURRENT = this;
}

void shader_program::unuse() const
{
	glUseProgram(0);
	if (_CURRENT == this)
		_CURRENT = nullptr;
}

bool shader_program::used() const
{
	GLint program_id = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program_id);
	return _id == program_id;
}

GLuint shader_program::attrib_location(char const * name) const
{
	return glGetAttribLocation(_id, name);
}

void shader_program::create_program_lazy()
{
	if (_id < 1)
		_id = glCreateProgram();

	if (_id < 1)
		throw shader_program_exception("unable to create shader program");
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
void primitive_uniform_upload<glm::mat3>(GLint location, glm::mat3 const & m)
{
	glUniformMatrix3fv(location, 1, GL_FALSE, &m[0][0]);
}

template <>
void primitive_uniform_upload<glm::mat4>(GLint location, glm::mat4 const & m)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
}

template <>
void primitive_uniform_upload<glm::vec2>(GLint location, glm::vec2 const & v)
{
	glUniform2fv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<glm::vec3>(GLint location, glm::vec3 const & v)
{
	glUniform3fv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<glm::vec4>(GLint location, glm::vec4 const & v)
{
	glUniform4fv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<glm::ivec2>(GLint location, glm::ivec2 const & v)
{
	glUniform2iv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<glm::ivec3>(GLint location, glm::ivec3 const & v)
{
	glUniform3iv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<glm::ivec4>(GLint location, glm::ivec4 const & v)
{
	glUniform4iv(location, 1, glm::value_ptr(v));
}

template <>
void primitive_uniform_upload<int>(GLint location, int const & v)
{
	glUniform1i(location, v);
}

template<>
void primitive_uniform_upload<float>(GLint location, float const & v)
{
	glUniform1f(location, v);
}

// nakresli trojuholnik
#include "program.hpp"
#include <iostream>
#include <memory>
#include <boost/filesystem/path.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "utils.hpp"

using std::string;

namespace shader {

void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

program * program::_CURRENT = nullptr;

namespace fs = boost::filesystem;

program::program(std::string const & fname)
{
	fs::path fpath("res/shaders");
	fpath /= fname;

	init(make_ptr<module>(read_file(fpath.string())));
}

program::program(ptr<module> m)
{
	init(m);
}

void program::init(ptr<module> m)
{
	_pid = glCreateProgram();

	for (unsigned sid : m->ids())
		glAttachShader(_pid, sid);

	glLinkProgram(_pid);

	bool result = link_check();
	assert(result && "program linking failed");
	if (!result)
		throw std::exception();  // TODO: specify exception

	_modules.push_back(m);

	init_uniforms();
}


program::~program()
{
	if (used())
		glUseProgram(0);

	_CURRENT = nullptr;

	glDeleteProgram(_pid);
}

void program::use()
{
	if (_CURRENT == this)
		return;

	glUseProgram(_pid);
	_CURRENT = this;
}

bool program::used() const
{
	return _CURRENT == this;
}

ptr<uniform> program::uniform_variable(std::string name)
{
	auto it = _uniforms.find(name);
	assert(it != _uniforms.end() && "unknown uniform variable (or not active)");
	return it->second;
}

void program::init_uniforms()
{
	GLint max_length = 0;
	glGetProgramiv(_pid, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);

	std::unique_ptr<GLchar[]> buf{new GLchar[max_length]};

	GLint nuniforms = 0;
	glGetProgramiv(_pid, GL_ACTIVE_UNIFORMS, &nuniforms);
	for (int i = 0; i < nuniforms; ++i)
	{
		GLint len = 0;
		glGetActiveUniformName(_pid, i, max_length, &len, buf.get());
		string uname(buf.get());
		append_uniform(uname, i);
	}
}

void program::append_uniform(std::string const & name, int index)
{
	ptr<uniform> u = make_ptr<uniform>(index, this);
	_uniforms[name] = u;
}

bool program::link_check()
{
	GLint linked;
	glGetProgramiv(_pid, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(_pid, "<unspecified>");
	return linked != GL_FALSE;
}

module::module(string const & code)
{
	_ids[0] = _ids[1] = 0;

	if (code.find("_VERTEX_") != string::npos)
		compile(code, shader_type::vertex);

	if (code.find("_FRAGMENT_") != string::npos)
		compile(code, shader_type::fragment);
}

module::~module()
{
	for (unsigned sid : _ids)
	{
		if (sid > 0)
			glDeleteShader(sid);
	}
}

boost::filtered_range<detail::valid_shader_pred, const unsigned[2]> module::ids() const
{
	detail::valid_shader_pred pred;
	return boost::filtered_range<detail::valid_shader_pred, const unsigned[2]>(pred, _ids);
}

void module::compile(std::string code, shader_type type)
{
	char const * lines[3];
	lines[0] = "#version 330\n";

	unsigned sid;
	GLenum stype;

	switch (type)
	{
		case shader_type::vertex:
			sid = 0;
			lines[1] = 	"#define _VERTEX_\n";
			stype = GL_VERTEX_SHADER;
			break;

		case shader_type::fragment:
			sid = 1;
			lines[1] = "#define _FRAGMENT_\n";
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

	return compiled != GL_FALSE;
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

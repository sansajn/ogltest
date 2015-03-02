#include "program.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/format.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

using std::string;
using std::ifstream;
using std::stringstream;
using std::shared_ptr;
using std::make_shared;

namespace shader {

string read_file(string const & fname);
void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

program * program::_CURRENT = nullptr;

program::program()
{
	_pid = glCreateProgram();
}

program::program(string const & fname)
{
	init(shared_ptr<module>(new module(fname)));
}

program::program(shared_ptr<module> m)
{
	init(m);
}

void program::read(string const & fname)
{
	init(shared_ptr<module>(new module(fname)));
}

void program::attach(std::shared_ptr<module> m)
{
	for (unsigned sid : m->ids())
		glAttachShader(_pid, sid);

	link();
	init_uniforms();

	_modules.push_back(m);
}

void program::attach(std::vector<std::shared_ptr<module>> const & mods)
{
	for (auto m : mods)
	{
		for (unsigned sid : m->ids())
			glAttachShader(_pid, sid);
	}

	link();
	init_uniforms();

	for (auto m : mods)
		_modules.push_back(m);
}

void program::init(shared_ptr<module> m)
{
	_pid = glCreateProgram();

	for (unsigned sid : m->ids())
		glAttachShader(_pid, sid);

	link();
	init_uniforms();

	_modules.push_back(m);
}

void program::link()
{
	glLinkProgram(_pid);

	bool result = link_check();
	assert(result && "program linking failed");
	if (!result)
		throw std::exception();  // TODO: specify exception
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

shared_ptr<uniform> program::uniform_variable(string const & name)
{
	auto it = _uniforms.find(name);

	if (it == _uniforms.end())
		throw exception(boost::str(boost::format("unknown (or not active) uniform variable '%1%'") % name));

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
	_uniforms[name] = make_shared<uniform>(index, this);
}

bool program::link_check()
{
	GLint linked;
	glGetProgramiv(_pid, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(_pid, "<unspecified>");
	return linked != GL_FALSE;
}

module::module(string const & fname, unsigned version)
{
	_ids[0] = _ids[1] = 0;
	_fname = fname;

	string code = read_file(fname);

	if (code.find("_VERTEX_") != string::npos)
		compile(version, code, shader_type::vertex);

	if (code.find("_FRAGMENT_") != string::npos)
		compile(version, code, shader_type::fragment);

	// TODO: ohandluj pripad, ked veni definovany ani _VERTEX_ ani _FRAGMENT_
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

void module::compile(unsigned version, std::string const & code, shader_type type)
{
	char const * lines[3];
	string vstr = string("#version ") + std::to_string(version) + string("\n");
	lines[0] = vstr.c_str();

	unsigned sid;
	GLenum stype;

	switch (type)
	{
		case shader_type::vertex:
			sid = 0;
			lines[1] = "#define _VERTEX_\n";
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
	string name = _fname;
	switch (type)
	{
		case shader_type::vertex:
			name += ":vertex";
			break;

		case shader_type::fragment:
			name += ":fragment";
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

string read_file(string const & fname)
{
	ifstream in(fname);
	assert(in.is_open() && "can't open file");
	if (!in.is_open())
		throw std::runtime_error(boost::str(boost::format("can't open '%1%' file") % fname));

	stringstream ss;
	ss << in.rdbuf();
	in.close();
	return ss.str();
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
void set_uniform<float>(int location, float const * a, int n)
{
	glUniform1fv(location, n, a);
}

template <>
void get_uniform(unsigned program, int location, int & v)
{
	glGetUniformiv(program, location, &v);
}

}  // shader

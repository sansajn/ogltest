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
string to_string(module::shader_type type);

program * program::_CURRENT = nullptr;

program::program() : _pid(0)
{}

program::program(string const & fname) : _pid(0)
{
	attach(shared_ptr<module>(new module(fname)));
}

program::program(shared_ptr<module> m) : _pid(0)
{
	attach(m);
}

void program::read(string const & fname)
{
	attach(shared_ptr<module>(new module(fname)));
}

void program::attach(std::shared_ptr<module> m)
{
	create_program_lazy();

	for (unsigned sid : m->ids())
		glAttachShader(_pid, sid);

	link();
	init_uniforms();

	_modules.push_back(m);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void program::attach(std::vector<std::shared_ptr<module>> const & mods)
{
	create_program_lazy();

	for (auto m : mods)
	{
		for (unsigned sid : m->ids())
			glAttachShader(_pid, sid);
	}

	link();
	init_uniforms();

	for (auto m : mods)
		_modules.push_back(m);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void program::create_program_lazy()
{
	if (_pid == 0)
		_pid = glCreateProgram();
	assert(_pid > 0 && "unable to create a program");
}

void program::link()
{
	glLinkProgram(_pid);

	bool result = link_check();
	assert(result && "program linking failed");
	if (!result)
		throw exception("unable to link a program");
}

program::~program()
{
	if (used())
	{
		glUseProgram(0);
		_CURRENT = nullptr;
	}

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
	_ids[0] = _ids[1] = _ids[2] = 0;
	_fname = fname;

	string code = read_file(fname);

	if (code.find("_VERTEX_") != string::npos)
		compile(version, code, shader_type::vertex);

	if (code.find("_FRAGMENT_") != string::npos)
		compile(version, code, shader_type::fragment);

	if (code.find("_GEOMETRY_") != string::npos)
		compile(version, code, shader_type::geometry);

	if (_ids[0] == 0 && _ids[1] == 0 && _ids[2] == 0)
		throw exception("empty shader module");
}

module::~module()
{
	for (unsigned sid : _ids)
	{
		if (sid > 0)
			glDeleteShader(sid);
	}
}

boost::filtered_range<detail::valid_shader_pred, const unsigned[int(module::shader_type::number_of_types)]> module::ids() const
{
	detail::valid_shader_pred pred;
	return boost::filtered_range<detail::valid_shader_pred, const unsigned[int(shader_type::number_of_types)]>(pred, _ids);
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

		case shader_type::geometry:
			sid = 2;
			lines[1] = "#define _GEOMETRY_\n";
			stype = GL_GEOMETRY_SHADER;
			break;

		default:
			assert(0 && "unsupported shader program type");
	}

	lines[2] = code.c_str();

	_ids[sid] = glCreateShader(stype);
	glShaderSource(_ids[sid], 3, lines, nullptr);
	glCompileShader(_ids[sid]);

	compile_check(_ids[sid], type);
}

void module::compile_check(unsigned sid, shader_type type)
{
	string name = _fname + string(":") + to_string(type);

	GLint compiled;
	glGetShaderiv(sid, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE)  // TODO: zobrazit warningi ?
	{
		dump_compile_log(sid, name);
		throw exception("program shader compilation failed");
	}

	assert(glGetError() == GL_NO_ERROR && "opengl error");
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

string to_string(module::shader_type type)
{
	switch (type)
	{
		case module::shader_type::vertex: return "vertex";
		case module::shader_type::fragment: return "fragment";
		case module::shader_type::geometry: return "geometry";

		default:
			throw exception("unsupported shader program type");
	}
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

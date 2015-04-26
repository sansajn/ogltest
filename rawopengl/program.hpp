#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <boost/range/adaptor/filtered.hpp>
#include <GL/glew.h>

namespace shader {

class program;

struct exception : public std::runtime_error
{
	exception(std::string const & msg) : std::runtime_error(msg) {}
};

class uniform
{
public:
	uniform(int loc, program * prog) : _loc(loc), _prog(prog) {}

	template <typename T>
	uniform & operator=(T const & v);

	template <typename T>
	operator T();

private:
	int _loc;
	program * _prog;
};

namespace detail {

struct valid_shader_pred
{
	bool operator()(unsigned const & v) const {return v > 0;}
};

}  // detail

class module
{
public:
	enum class shader_type
	{
		vertex,
		fragment,
		geometry,
		number_of_types
	};

	module(std::string const & fname, unsigned version = 330);
	~module();

	boost::filtered_range<detail::valid_shader_pred, const unsigned[int(shader_type::number_of_types)]> ids() const;

private:
	void compile(unsigned version, std::string const & code, shader_type type);
	void compile_check(unsigned sid, shader_type type);

	unsigned _ids[int(shader_type::number_of_types)];  //!< (vertex, fragment, geometry) shader id

	// debug
	std::string _fname;
};

class program
{
public:
	program();
	program(std::string const & fname);
	program(std::shared_ptr<module> m);
	~program();

	void read(std::string const & fname);
	void attach(std::shared_ptr<module> m);
	void attach(std::vector<std::shared_ptr<module>> const & mods);

	int id() const {return _pid;}
	void use();
	bool used() const;

	std::shared_ptr<uniform> uniform_variable(std::string const & name);

	template <typename T>
	void uniform_variable(std::string const & name, T const & v);

	program(program &) = delete;
	void operator=(program &) = delete;

private:
	void create_program_lazy();
	void init_uniforms();
	void append_uniform(std::string const & name, int index);
	void link();
	bool link_check();

	unsigned _pid;  //!< progrm id
	std::vector<std::shared_ptr<module>> _modules;
	std::map<std::string, std::shared_ptr<uniform>> _uniforms;

	static program * _CURRENT;
};

template <typename T>
void set_uniform(int location, T const & v);

template <typename T>
void set_uniform(int location, T const * a, int n);

template <typename T>
void set_uniform(int location, std::pair<T *, int> const & a)
{
	set_uniform(location, a.first, a.second);
}

template <typename T>
void get_uniform(unsigned program, int location, T & v);

template <typename T>
uniform & uniform::operator=(T const & v)
{
	assert(_prog->used() && "pokusam sa nastavit uniform neaktivneho programu");
	set_uniform(_loc, v);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
	return *this;
}

template <typename T>
uniform::operator T()
{
	T v;
	get_uniform(_prog->id(), _loc, v);
	return v;
}

template <typename T>
void program::uniform_variable(std::string const & name, T const & v)
{
	auto u = uniform_variable(name);
	*u = v;
}

}  // shader

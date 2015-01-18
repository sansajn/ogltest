#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <cassert>
#include <boost/range/adaptor/filtered.hpp>
#include "ptr.hpp"

namespace shader {

struct exception : public std::runtime_error
{
	exception(std::string const & msg) : std::runtime_error(msg) {}
};

class program;

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
	module(std::string const & code);
	~module();

	boost::filtered_range<detail::valid_shader_pred, const unsigned[2]> ids() const;

private:
	enum class shader_type
	{
		vertex,
		fragment
	};

	void compile(std::string code, shader_type type);
	bool compile_check(unsigned sid, shader_type type);

	unsigned _ids[2];  //!< (vertex, fragment) shader id
};

class program
{
public:
	program(std::string const & fname);
	program(ptr<module> m);
	~program();

	int id() const {return _pid;}
	void use();
	bool used() const;
	ptr<uniform> uniform_variable(std::string const & name);

private:
	void init(ptr<module> m);
	void init_uniforms();
	void append_uniform(std::string const & name, int index);
	bool link_check();

	unsigned _pid;  //!< progrm id

	std::vector<ptr<module>> _modules;
	std::map<std::string, ptr<uniform>> _uniforms;

	static program * _CURRENT;
};

template <typename T>
void set_uniform(int location, T const & v);

template <typename T>
void get_uniform(unsigned program, int location, T & v);

template <typename T>
uniform & uniform::operator=(T const & v)
{
	assert(_prog->used() && "pokusam sa nastavit uniform neaktivneho programu");
	set_uniform(_loc, v);
	return *this;
}

template <typename T>
uniform::operator T()
{
	T v;
	get_uniform(_prog->id(), _loc, v);
	return v;
}

}  // shader

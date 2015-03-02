#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <boost/range/adaptor/filtered.hpp>

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
	module(std::string const & fname, unsigned version = 330);
	~module();

	boost::filtered_range<detail::valid_shader_pred, const unsigned[2]> ids() const;

private:
	enum class shader_type
	{
		vertex,
		fragment
	};

	void compile(unsigned version, std::string const & code, shader_type type);
	bool compile_check(unsigned sid, shader_type type);

	unsigned _ids[2];  //!< (vertex, fragment) shader id

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

private:
	void init(std::shared_ptr<module> m);
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

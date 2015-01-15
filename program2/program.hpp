#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <cassert>

namespace shader {

template <typename T>
using ptr = std::shared_ptr<T>;

template <typename T, typename ... Args>
ptr<T> make_ptr(Args && ... args) {return std::make_shared<T>(args ...);}

template <typename T>
void set_uniform(int location, T const & v);

template <typename T>
void get_uniform(unsigned program, int location, T & v);

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

class module
{
public:
	module(std::string code);

	unsigned const * ids() const {return _ids;}  // TODO: implementuj range

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
	program(ptr<module> m);

	int id() const {return _pid;}
	void use();
	bool used();
	ptr<uniform> uniform_variable(std::string name);

private:
	bool link_check();

	unsigned _pid;  //!< progrm id

	std::vector<ptr<module>> _modules;
//	std::map<string, ptr<uniform>> _uniforms;
};

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

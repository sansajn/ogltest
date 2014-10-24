#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <GL/glew.h>
#include "core/ptr.hpp"

struct shader_program_exception : public std::runtime_error
{
	shader_program_exception(std::string const & msg) : std::runtime_error(msg)
	{}
};

class shader_program;

/*! Uniform representation.
\ingroup render
\code
uniform_variable u("scale", prog);
u = 12.43f;

uniform_variable u;
u.link("scale", prog);
u = 12.43f;
\code */
class uniform_variable
{
public:
	uniform_variable() : _location(0) {}
	uniform_variable(char const * name, shader_program const & prog);
	virtual ~uniform_variable() {}
	void link(const char * name, shader_program const & prog);
	GLint location() const {return _location;}

	template <typename T>
	uniform_variable & operator=(T const & v);

private:
	GLint _location;
};

// TODO: definuj typi modulu (shader_type)

class shader_module
{
public:
	shader_module(char const * fname);
	shader_module(char const * fname, GLenum type);
	~shader_module();
	GLuint id() const {return _id;}
	GLenum type() const {return _type;}

private:
	void compile(char const * filename, GLenum type);

	GLuint _id;
	GLenum _type;
};

/*! Shader program representation.
\ingroup render
\code
shader_program prog;
prog << "basic.vs" << "plastic.fs";  // load and compile shader modules
prog.link();
prog.use();
uniform_variable u("color", program);
u = glm::vec4(1.0, 0.0, 0.0, 1.0);
render();
prog.unuse();
\endcode */
class shader_program
{
public:
	shader_program();
	~shader_program();

	void attach(ptr<shader_module> module);
	void attach(char const * fname);
	void attach(char const * fname, GLenum type);
	void link();
	bool linked() const {return _linked;}	
	void use() const;
	bool used() const;
	void unuse() const;
	GLuint id() const {return _id;}

	GLuint attrib_location(char const * name) const;  // TODO: oddelit rovnako ako uniform

	static shader_program const * current_used_program() {return _CURRENT;}

private:
	void create_program_lazy();

	GLuint _id;
	bool _linked;
	std::vector<ptr<shader_module>> _modules;

	static shader_program const * _CURRENT;  //! shader program in use
};  // shader_program


// attach shortcuts
inline shader_program & operator<<(shader_program & prog, char const * filename)
{
	prog.attach(filename);
	return prog;
}

inline shader_program & operator<<(shader_program & prog, std::string const & filename)
{
	return (prog << filename.c_str());
}

inline shader_program & operator<<(shader_program & prog, ptr<shader_module> module)
{
	prog.attach(module);
	return prog;
}


template <typename T>
void primitive_uniform_upload(GLint location, T const & v);

template <typename T>
uniform_variable & uniform_variable::operator=(T const & v)
{
	primitive_uniform_upload(_location, v);
	return *this;
}

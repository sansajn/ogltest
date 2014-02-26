#pragma once

#include <string>
#include <stdexcept>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>


namespace gl {

struct program_exception : public std::runtime_error
{
	program_exception(std::string const & msg) : std::runtime_error(msg)
	{}
};

class program
{
public:
	program();
	~program();
	
	void compile(char const * filename);
	void compile(char const * filename, GLenum type);
	
	void link();
	bool linked() const {return _linked;}
	
	void use() const;
	void unuse() const;
	bool used() const;

	template <typename T>
	void uniform(char const * name, T const & v);

	void sampler_uniform(char const * name, int texture_unit);  //!< just as texture-unit remainder

	// cooperation with raw ogl
	GLuint id() const {return _program;}
	GLuint attrib_location(char const * name) const;

private:
	std::string read_shader(char const * filename);
	void create_program_lazy();
	GLint uniform_location(char const * name);

	GLuint _program;
	std::map<std::string, GLint> _uniforms;
	bool _linked;
};

// compile shortcut
inline program & operator<<(program & prog, char const * filename)
{
	prog.compile(filename);
	return prog;
}

template <typename T>
void uniform_upload(GLuint location, T const & v);

template <typename T>
void program::uniform(char const * name, T const & v)
{
	if (!used())
		throw program_exception("program is not used");

	uniform_upload(uniform_location(name), v);
}

};  // gl

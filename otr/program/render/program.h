#pragma once
#include <string>
#include <stdexcept>
#include <map>
#include <GL/glew.h>


struct program_exception : public std::runtime_error
{
	program_exception(std::string const & msg) : std::runtime_error(msg)
	{}
};

//! Uniform representation.
class uniform_variable
{
public:
	uniform_variable(GLint location) : _location(location) {}

	template <typename T>
	uniform_variable & operator=(T const & v);

private:
	GLint _location;
};

/*! Shader program representation.
\code
gl::shader_program prog;
prog << "basic.vs" << "plastic.fs";  // load and compile shader modules
prog.link();
prog.use();
prog.uniform("color", glm::vec4(1.0, 0.0, 0.0, 1.0));
\endcode */
class shader_program
{
public:
	shader_program();
	~shader_program();

	void compile(char const * filename);
	void compile(char const * filename, GLenum type);

	void link();
	bool linked() const {return _linked;}	

	void use() const;
	bool used() const;
	void unuse() const;

	uniform_variable & uniform(char const * name);

	template <typename T>
	void uniform(char const * name, T const & v) {uniform(name) = v;}

	void sampler_uniform(char const * name, int texture_unit);  //!< just as texture-unit remainder

	// cooperation with raw ogl
	GLuint id() const {return _program;}
	GLuint attrib_location(char const * name) const;

private:
	std::string read_shader(char const * filename);
	void create_program_lazy();

	GLuint _program;
	std::map<std::string, uniform_variable> _uniforms;
	bool _linked;
};

// compile shortcut
inline shader_program & operator<<(shader_program & prog, char const * filename)
{
	prog.compile(filename);
	return prog;
}

inline shader_program & operator<<(shader_program & prog, std::string const & filename)
{
	return (prog << filename.c_str());
}

template <typename T>
void uniform_upload(GLuint location, T const & v);


template <typename T>
uniform_variable & uniform_variable::operator=(T const & v)
{
	uniform_upload(_location, v);
	return *this;
}

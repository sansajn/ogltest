#pragma once
#include "gl/program.hpp"
#include "gl/camera.hpp"
#include "gl/mesh.hpp"

namespace shader {

/*! Implementuje shader s phongovim osvetlovacim modelom.
\code
	shader::program prog;
	prog.from_memory(shader::phong_shader_source);
	shader::phong_light phong;
	phong.init(&prog, &cam);
	// ...
	phong.render(m, mat4{1});
\endcode */
class phong_light
{
public:
	phong_light() {}
	void init(shader::program * p, gl::camera const * cam);
	void render(gl::mesh const & m, glm::mat4 const & local_to_world);
	void attach_camera(gl::camera const * cam);
	void light(glm::vec3 const & direction, glm::vec3 const & color, float intensity);  //!< \param[in] direction ide o normalizovany vektor
	void material(glm::vec3 const & ambient, float shininess, float intensity);
	void light_direction(glm::vec3 const & dir);
	shader::program & shader_program() {return *_prog;}

private:
	shader::program * _prog = nullptr;
	gl::camera const * _cam = nullptr;
};

// predefined shader sources
extern char const * phong_shader_source;

}  // shader

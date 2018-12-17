#include "gles2/light_gles2.hpp"

namespace gles2 {

phong_light::phong_light(glm::vec3 const & direction, glm::vec3 const & color, float intensity)
	: direction{direction}, color{color}, intensity{intensity}
{}

void phong_light::apply(shader::program & p)
{
	p.uniform_variable("light_direction", direction);
	p.uniform_variable("light_color", color);
	p.uniform_variable("light_intensity", intensity);
}


diffuse_light::diffuse_light(glm::vec3 const & direction)
	: direction{direction}
{}

void diffuse_light::apply(shader::program & p)
{
	p.uniform_variable("light_dir", direction);
}

}  // gl

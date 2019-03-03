#include "gles2/default_shader_gles2.hpp"
#include "shaded_shader.hpp"

namespace gles2 {

using glm::mat4;
using glm::vec3;
using glm::mat3;

shaded_shader::shaded_shader()
	: _position_a{-1}, _normal_a{-1}
{}

shaded_shader::shaded_shader(vec3 const & object_color, vec3 const & light_dir)
	: _color{object_color}, _light_dir{light_dir}
{}

void shaded_shader::render(mesh & m, mat4 const & M, mat3 const & normal_to_world)
{
	assert(_prog.used() && "program needs to be used to update uniforms");
	*_local_to_screen_u = _world_to_screen * M;
	*_normal_to_world_u = normal_to_world;

	m.attribute_location({_position_a, _normal_a});

	m.render();
}

void shaded_shader::update_properties()
{
	*_color_u = _color;
	*_light_dir_u = _light_dir;
}

void shaded_shader::object_color(vec3 const & c)
{
	_color = c;
}

void shaded_shader::light_direction(vec3 const & d)
{
	_light_dir = d;
}

void shaded_shader::view_projection(mat4 const & VP)
{
	_world_to_screen = VP;
}

void shaded_shader::use()
{
	if (_prog.id() == 0)  // lazy create
		_prog.from_memory(gles2::flat_shaded_shader_source, 100);

	_local_to_screen_u = _prog.uniform_variable("local_to_screen");
	_normal_to_world_u = _prog.uniform_variable("normal_to_world");
	_color_u = _prog.uniform_variable("color");
	_light_dir_u = _prog.uniform_variable("light_dir");

	_position_a = _prog.attribute_location("position"),
	_normal_a = _prog.attribute_location("normal");

	_prog.use();
}

}  // gles2

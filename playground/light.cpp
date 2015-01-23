#include "light.hpp"

ambient_light::ambient_light()
	: game_shader("amblight.glsl"), _ambient(glm::vec3(0.1, 0.1, 0.1))
{}

void ambient_light::update_uniforms(material const & mat, game_object & obj)
{
	_prog.use();

	_prog.uniform_variable("local_to_screen", obj.local_to_screen());

	glm::vec3 ambient;
	if (!mat.find("ambient", ambient))
		ambient = _ambient;
	_prog.uniform_variable("ambient", ambient);

	mat.get_texture("diffuse")->bind(0);
	_prog.uniform_variable("diffuse", 0);
}

void directional_light::update_uniforms(material const & mat, game_object & obj)
{
	_prog.use();

	_prog.uniform_variable("local_to_world", obj.local_to_world());
	_prog.uniform_variable("local_to_screen", obj.local_to_screen());
	_prog.uniform_variable("camera_pos", obj.owner().camera_position());

	_prog.uniform_variable("light.color", glm::vec3(1,1,1));
	_prog.uniform_variable("light.intensity", 0.7f);
	_prog.uniform_variable("light.direction", glm::normalize(glm::vec3(0,1,1)));

	_prog.uniform_variable("spec_intensity", mat.get_float("specular_intensity"));
	_prog.uniform_variable("spec_power", mat.get_float("specular_power"));

	mat.get_texture("diffuse")->bind(0);
	_prog.uniform_variable("diffuse", 0);
}

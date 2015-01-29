#include "light.hpp"
#include <glm/gtc/matrix_inverse.hpp>

ambient_shader::ambient_shader()
	: game_shader("ambient.glsl"), _ambient(glm::vec3(0.1, 0.1, 0.1))
{}

void ambient_shader::update_uniforms(material const & mat, game_object & obj)
{
	_prog.use();

	glm::mat4 M = obj.local_to_world();
	glm::mat4 V = obj.owner().camera_object()->world_to_local();
	glm::mat4 P = obj.owner().camera_to_screen();
	glm::mat3 N = glm::mat3(glm::inverseTranspose(M*V));

	_prog.uniform_variable("camera_to_screen", P);
	_prog.uniform_variable("world_to_camera", V);
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", N);

	glm::vec3 ambient;
	if (!mat.find("ambient", ambient))
		ambient = _ambient;
	_prog.uniform_variable("material.ambient", ambient);

	_prog.uniform_variable("parallax_scale_bias", glm::vec2(mat.get_float("disp_scale"), mat.get_float("disp_bias")));

	mat.get_texture("diffuse")->bind(0);
	_prog.uniform_variable("diffuse", 0);

	mat.get_texture("heightmap")->bind(2);
	_prog.uniform_variable("diffuse", 2);
}

directional_shader::directional_shader(glm::vec3 const & color, float intensity)
	: game_shader("dirlight.glsl"), _color(color), _intensity(intensity)
{}

void directional_shader::update_uniforms(material const & mat, game_object & obj)
{
	_prog.use();

	glm::mat4 M = obj.local_to_world();
	glm::mat4 V = obj.owner().camera_object()->world_to_local();
	glm::mat4 P = obj.owner().camera_to_screen();
	glm::mat3 N = glm::mat3(glm::inverseTranspose(M*V));

	_prog.uniform_variable("camera_to_screen", P);
	_prog.uniform_variable("world_to_camera", V);
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", N);

	_prog.uniform_variable("light.direction", glm::vec3(0,-1,-1));  // TODO: implementuj zmenu polohy svetla
	_prog.uniform_variable("light.color", _color);
	_prog.uniform_variable("light.intensity", _intensity);

	_prog.uniform_variable("material.shininess", mat.get_float("specular_power"));
	_prog.uniform_variable("material.intensity", mat.get_float("specular_intensity"));

	_prog.uniform_variable("parallax_scale_bias", glm::vec2(mat.get_float("disp_scale"), mat.get_float("disp_bias")));

	mat.get_texture("diffuse")->bind(0);
	_prog.uniform_variable("diffuse", 0);

	mat.get_texture("normalmap")->bind(1);
	_prog.uniform_variable("normalmap", 1);

	mat.get_texture("heightmap")->bind(2);
	_prog.uniform_variable("heightmap", 2);
}

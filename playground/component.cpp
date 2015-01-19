#include "component.hpp"
#include <glm/gtc/matrix_transform.hpp>

camera::camera(float fov, float aspect, float near, float far)
	: _P(glm::perspective(fov, aspect, near, far))
{}

glm::mat4 camera::view_projection() const
{
	glm::mat4 R = glm::mat4_cast(glm::conjugate(transformation().rotation));
	glm::mat4 T = glm::translate(glm::mat4(1.0f), -transformation().position);
	return _P * R * T;  // je to inverzna transformacia, preto R*T (najprv posunie, potom odrotuje)
}

glm::vec3 camera::right() const
{
	glm::mat3 R = glm::mat3_cast(transformation().rotation);
	return R[0];
}

glm::vec3 camera::up() const
{
	glm::mat3 R = glm::mat3_cast(transformation().rotation);
	return R[1];
}

glm::vec3 camera::forward() const
{
	glm::mat3 R = glm::mat3_cast(transformation().rotation);
	return R[2];
}

material::material(ptr<texture> diffuse, float specular_intensity, float specular_power)
{
	assoc_texture("diffuse", diffuse);
	assoc_float("specular_intensity", specular_intensity);
	assoc_float("specular_power", specular_power);
}

float material::get_float(std::string const & name)
{
	auto it = _floats.find(name);
	assert(it != _floats.end() && "undefined property");
	return it->second;
}

glm::vec3 material::get_vector(std::string const & name)
{
	auto it = _vectors.find(name);
	assert(it != _vectors.end() && "undefined property");
	return it->second;
}

ptr<texture> material::get_texture(std::string const & name)
{
	auto it = _textures.find(name);
	assert(it != _textures.end() && "undefined property");
	return it->second;
}

void mesh_renderer::render(shader::program & prog, renderer & rend)
{
	prog.use();

	prog.uniform_variable("M", _owner->local_to_camera());
	prog.uniform_variable("MVP", _owner->local_to_screen());

	_material->get_texture("diffuse")->bind(0);
	auto diffuse = prog.uniform_variable("diffuse");
	*diffuse = 0;

	auto spec_intensity = prog.uniform_variable("material_specular_intensity");
	*spec_intensity = _material->get_float("specular_intensity");

	auto spec_power = prog.uniform_variable("material_specular_power");
	*spec_power = _material->get_float("specular_power");

	_mesh->draw();
}

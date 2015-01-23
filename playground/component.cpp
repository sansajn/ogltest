#include "component.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "light.hpp"

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

void camera::append_to_engine(engine & e)
{
	e.camera_object(this->_owner, _P);
}

void mesh_renderer::render(game_shader & shader, renderer & rend)
{
	shader.update_uniforms(*_material, *_owner);
	_mesh->draw();
}

directional_light::directional_light(glm::vec3 const & color, float intensity)
	: _color(color), _intensity(intensity)
{}

void directional_light::append_to_engine(engine & e)
{
	e.append_light(new directional_shader(_color, _intensity));
}

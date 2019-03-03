#pragma once
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"

namespace gles2 {

/* properties: object_color, light_direction */
class shaded_shader
{
public:
	shaded_shader();
	shaded_shader(glm::vec3 const & object_color, glm::vec3 const & light_dir);
	void use();
	void object_color(glm::vec3 const & c);
	void light_direction(glm::vec3 const & d);
	void update_properties();
	void view_projection(glm::mat4 const & VP);
	void render(mesh & m, glm::mat4 const & M, glm::mat3 const & normal_to_world);

private:
	glm::vec3 _color, _light_dir;
	glm::mat4 _world_to_screen;
	std::shared_ptr<shader::uniform> _local_to_screen_u;
	std::shared_ptr<shader::uniform> _normal_to_world_u;
	std::shared_ptr<shader::uniform> _color_u;
	std::shared_ptr<shader::uniform> _light_dir_u;
	int _position_a, _normal_a;
	shader::program _prog;
};

}  // gles2

#pragma once
#include <glm/vec3.hpp>
#include "program.hpp"
#include "engine.hpp"

class ambient_light : public game_shader
{
public:
	ambient_light();
	void update_uniforms(material const & mat, game_object & obj) override;

private:
	glm::vec3 _ambient;  //!< global ambient value
};

class directional_light : public game_shader
{
public:
	directional_light() : game_shader("dirlight.glsl") {}
	void update_uniforms(material const & mat, game_object & obj) override;
};

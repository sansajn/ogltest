// implementuje shader prograny s roznym typom osvetlenia
#pragma once
#include <glm/vec3.hpp>
#include "program.hpp"
#include "engine.hpp"

class ambient_shader : public game_shader
{
public:
	ambient_shader();
	void update_uniforms(material const & mat, game_object & obj) override;

private:
	glm::vec3 _ambient;  //!< global ambient value
};

class directional_shader : public game_shader
{
public:
	directional_shader(glm::vec3 const & color, float intensity);
	void update_uniforms(material const & mat, game_object & obj) override;

private:
	glm::vec3 _color;
	float _intensity;
};

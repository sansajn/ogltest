// casto pouzivane objekty v scene
#pragma once
#include <glm/matrix.hpp>
#include "gl/program.hpp"
#include "gl/colors.hpp"

class axis_object
{
public:
	axis_object();
	void render(glm::mat4 const & world_to_screen);

private:
	shader::program _prog;  // TODO: make shareable
};

class light_object
{
public:
	light_object(glm::vec3 const & color = rgb::yellow);
	void render(glm::mat4 const & local_to_screen);

private:
	glm::vec3 _color;
	shader::program _prog;  // TODO: make shareable
};

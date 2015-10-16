// casto pouzivane objekty v scene
#pragma once
#include <glm/vec3.hpp>
#include "program.hpp"
#include "camera.hpp"
#include "colors.hpp"

class axis_object
{
public:
	axis_object();
	void render(gl::camera & c);

private:
	shader::program _prog;
};

class light_object
{
public:
	light_object(glm::vec3 const & color = rgb::yellow);
	void render(gl::camera const & c, glm::vec3 const & position);

private:
	glm::vec3 _color;
	shader::program _prog;
};

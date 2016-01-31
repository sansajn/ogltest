#include "touch_button.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "gl/colors.hpp"
#include "gl/shapes.hpp"

namespace ui { namespace touch {

using std::shared_ptr;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::ivec2;
using glm::length;
using glm::translate;
using glm::scale;
using glm::ortho;
using gl::mesh;
using gl::make_circle_xy;

mat4 calculate_transform(ivec2 const & origin, unsigned radius, unsigned w, unsigned h);

void button::init(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h, shared_ptr<shader::program> prog)
{
	_origin = origin;
	_radius = radius;
	_prog = prog;
	_up = _down = false;
	_circle = make_circle_xy<mesh>();
	_local_to_screen = calculate_transform(origin, radius, screen_w, screen_h);
}

void button::update()
{
	_up = false;
}

void button::render()
{
	_prog->use();
	_prog->uniform_variable("local_to_screen", _local_to_screen);
	_prog->uniform_variable("color", rgb::white);
	_circle.render();
}

void button::screen_geometry(unsigned width, unsigned height)
{
	_local_to_screen = calculate_transform(_origin, _radius, width, height);
}

bool button::touch(glm::ivec2 const & pos, touch_event event)
{
	bool inside_circle = length(vec2{_origin - pos}) < _radius;

	// NOTE: generovat up mozem iba pre event vkruhu, v opacnompripade potrebujem sledovat id fingera

	if (!inside_circle)
		return false;

	if (event == touch_event::down)
	{
		_up = false;
		_down = true;
	}
	else if (event == touch_event::up)
	{
		_up = true;
		_down = false;
	}

	return true;
}

bool button::up() const
{
	return _up;
}

bool button::down() const
{
	return _down;
}

mat4 calculate_transform(ivec2 const & origin, unsigned radius, unsigned w, unsigned h)
{
	mat4 T = ortho(0.0f, (float)w, (float)h, 0.0f, 0.0f, 100.0f);
	T = translate(T, vec3{origin, 0});
	T = scale(T, vec3{(float)radius});
	return T;
}

}}  // ui::touch

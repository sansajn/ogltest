#include "touch_button_gles2.hpp"
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
using gles2::mesh;
using gles2::shader::program;
using gl::make_circle_xy;

mat4 calculate_transform(ivec2 const & origin, unsigned radius, unsigned w, unsigned h);

void button::init(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h, shared_ptr<program> prog)
{
	_origin = origin;
	_radius = radius;
	_prog = prog;
	_up = _down = false;
	_down_finger_id = -1;
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

bool button::evaluate_finger(ui::touch::finger const & f)
{
	bool inside_circle = length(vec2{_origin - f.position}) < _radius;  // TODO: neodmocnuj

	if (_down_finger_id != -1)  // uz sledujem prst
	{
		if (_down_finger_id != f.id)
			return false;  // ignoruj vsetky ostatne prsty

		if (!inside_circle)  // mimo kruhu, zrus down (negeneruj up)
		{
			_down = false;
			_down_finger_id = -1;
			return false;
		}

		if (f.up())
		{
			_up = true;
			_down = false;
			_down_finger_id = -1;
			return true;
		}
	}
	else  // este nesledujem ziaden prst
	{
		if (!inside_circle)
			return false;

		if (f.down())
		{
			_up = false;
			_down = true;
			_down_finger_id = f.id;
			return true;
		}
	}

	return false;
}

}}  // ui::touch

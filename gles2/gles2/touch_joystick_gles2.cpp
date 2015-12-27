#include "touch_joystick_gles2.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "gl/shapes.hpp"
#include "gl/colors.hpp"

using glm::mat4;
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::ortho;
using glm::orientedAngle;
using glm::translate;
using glm::scale;
using glm::length;
using glm::normalize;
using gles2::mesh;
using gl::make_circle_xy;

namespace gles2 {
	namespace ui {
		namespace touch {

namespace detail {

char const * flat_shader_source = R"(
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(.7);
	#ifdef _VERTEX_
	attribute vec3 position;
	void main() {
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	void main() {
		gl_FragColor = vec4(color, 1);
	}
	#endif
)";

}  // detail

joystick::joystick(ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h)
	: _origin{origin}, _radius{radius}, _inner_origin{origin}
{
	set_dirs(false, false, false, false);

	calculate_projection(screen_w, screen_h);
	mat4 T = translate(vec3{_origin, 0});
	mat4 S = scale(vec3(_radius));
	_local_to_camera = T * S;

	_circle = make_circle_xy<mesh>();
	_solid.from_memory(detail::flat_shader_source);
}

void joystick::render()
{
	_solid.use();
	_solid.uniform_variable("local_to_screen", _proj * _local_to_camera);
	_solid.uniform_variable("color", rgb::white);
	_circle.render();  // outer circle

	mat4 T = translate(vec3{_inner_origin, 0});
	mat4 S = scale(vec3{_radius/3.0f});
	_solid.uniform_variable("local_to_screen", _proj * T * S);
	_circle.render();  // inner circle
}

void joystick::touch(ivec2 const & pos, touch_event event)
{
	if (event == touch_event::up)
	{
		set_dirs(false, false, false, false);
		_inner_origin = _origin;
		return;
	}

	if (length(vec2{_origin - pos}) > _radius)
		return;  // som mimo kruhu

	// v kruhu mam 8 smerou pohybu hore, dole, lavo, vpravo, hore-vlavo, hore-vpravo, dole-vlavo, dole-vpravo
	float phase = M_PI/8;
	float dir_angle = M_PI/4;

	// spocitaj uhol miesta dotyku s osou x
	vec2 u{1,0};
	vec2 v = normalize(vec2{pos - _origin});
	v.y = -v.y;  // origin je lavo-hore
	float ang = orientedAngle(u,v);
	if (ang < 0)
		ang = 2*M_PI + ang;

	if (ang >= phase && ang < (phase+dir_angle))  // up-right
		set_dirs(true, false, false, true);
	else if (ang >= (phase+dir_angle) && ang < (phase+2*dir_angle))  // up
		set_dirs(true, false, false, false);
	else if (ang >= (phase+2*dir_angle) && ang < (phase+3*dir_angle))  // up-left
		set_dirs(true, false, true, false);
	else if (ang >= (phase+3*dir_angle) && ang < (phase+4*dir_angle))  // left
		set_dirs(false, false, true, false);
	else if (ang >= (phase+4*dir_angle) && ang < (phase+5*dir_angle))  // down-left
		set_dirs(false, true, true, false);
	else if (ang >= (phase+5*dir_angle) && ang < (phase+6*dir_angle))  // down
		set_dirs(false, true, false, false);
	else if (ang >= (phase+6*dir_angle) && ang < (phase+7*dir_angle))  // down-right
		set_dirs(false, true, false, true);
	else if (ang >= (phase+7*dir_angle) || ang < phase)  // right
		set_dirs(false, false, false, true);
	else
		assert(0 && "out of angle");

	_inner_origin = pos;
}

bool joystick::up() const
{
	return _dirs[0];
}

bool joystick::down() const
{
	return _dirs[1];
}

bool joystick::left() const
{
	return _dirs[2];
}

bool joystick::right() const
{
	return _dirs[3];
}

void joystick::screen_geometry(unsigned width, unsigned height)
{
	calculate_projection(width, height);
}

void joystick::set_dirs(bool up, bool down, bool left, bool right)
{
	_dirs[0] = up;
	_dirs[1] = down;
	_dirs[2] = left;
	_dirs[3] = right;
}

void joystick::calculate_projection(unsigned screen_w, unsigned screen_h)
{
	_proj = ortho(0.0f, (float)screen_w, (float)screen_h, 0.0f, 0.0f, 100.0f);
}

		}  // touch
	}  // ui
}  // gles2

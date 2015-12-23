// test implementacie joysticku pre touch-screen zariadenia
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "gl/program.hpp"
#include "gl/colors.hpp"
#include "gl/glut_window.hpp"
#include "gl/touch_joystick.hpp"

using glm::mat4;
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::scale;
using glm::translate;
using glm::ortho;


char const * solid_shader_source = R"(
	// #version 330
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(.7);
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	void main() {
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main() {
		fcolor = vec4(color, 1);
	}
	#endif
)";


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void input(float dt) override;
	void display() override;

private:
	gl::mesh _quad;
	shader::program _solid;
	ui::touch::joystick _joystick;
	vec2 _quad_pos;
};

scene_window::scene_window() : _joystick{vec2{100, 500}, 50, width(), height()}
{
	_quad = gl::make_quad_xy<gl::mesh>();
	_solid.from_memory(solid_shader_source);
	_quad_pos = vec2{width()/2, height()/2};
	glClearColor(0,0,0,1);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	_solid.use();
	mat4 P = ortho(0.0f, (float)width(), (float)height(), 0.0f, 0.0f, 1000.0f);
	mat4 T = translate(vec3{_quad_pos, 0});
	mat4 S = scale(vec3{30});
	mat4 local_to_screen = P * T * S;
	_solid.uniform_variable("local_to_screen", local_to_screen);
	_solid.uniform_variable("color", rgb::purple);
	_quad.render();
	_joystick.render();
	base::display();
}

void scene_window::input(float dt)
{
	base::input(dt);

	// resolve joystick input
	glm::ivec2 mouse_pos = in.mouse_position();
	if (in.mouse(button::left))
		_joystick.touch(vec2{mouse_pos.x, mouse_pos.y}, ui::touch::joystick::touch_event::down);
	else
		_joystick.touch(vec2{mouse_pos.x, mouse_pos.y}, ui::touch::joystick::touch_event::up);

	float linear_velocity = 100;  // in pixels
	if (_joystick.up())
		_quad_pos += vec2{0, -dt*linear_velocity};
	if (_joystick.down())
		_quad_pos += vec2{0, dt*linear_velocity};
	if (_joystick.left())
		_quad_pos += vec2{-dt*linear_velocity, 0};
	if (_joystick.right())
		_quad_pos += vec2{dt*linear_velocity, 0};
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

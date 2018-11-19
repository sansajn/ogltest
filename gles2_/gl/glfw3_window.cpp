#include "glfw3_window.hpp"
#include <GLFW/glfw3.h>

#include <iostream>

namespace ui {

using glm::ivec2;

namespace glfw_detail {

glfw3_layer * active_window();
event_handler::modifier to_modifier(int m);

void keyboard_func(GLFWwindow * window, int key, int scancode, int action, int mods);

glfw3_layer * __window = nullptr;
GLFWwindow * __glfw_window = nullptr;

}  // glfw_detail


glfw3_layer::glfw3_layer(parameters const & params)
{
	assert(!glfw_detail::__window && "GLFW3 already initialized");

	if (!glfw_detail::__window)
	{
		glfwInit();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	glfw_detail::__window = this;

	GLFWwindow * window = glfwCreateWindow(params.width(), params.height(), __FILE__, NULL, NULL);
	assert(window);

	glfwSetKeyCallback(window, glfw_detail::keyboard_func);

	glfwMakeContextCurrent(window);
	glfw_detail::__glfw_window = window;
}

glfw3_layer::~glfw3_layer()
{
	glfwTerminate();
	glfw_detail::__glfw_window = nullptr;
}

void glfw3_layer::display()
{
	glfwSwapBuffers(glfw_detail::__glfw_window);
}

void glfw3_layer::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void glfw3_layer::main_loop_event()
{
	glfwPollEvents();
	if (glfwWindowShouldClose(glfw_detail::__glfw_window))
		close();
}


glfw3_layer::user_input::user_input()
{
	for (unsigned i = 0; i < NUM_KEYS; ++i)
		_keys[i] = _keys_up[i] = false;

	_mouse_pos = glm::vec2(0,0);  // TODO: zvaz hodnotu center
	for (unsigned i = 0; i < NUM_BUTTONS; ++i)
		_mouse_buttons[i] = _mouse_buttons_up[i] = false;
}

bool glfw3_layer::user_input::key(unsigned char c) const
{
	return _keys[c];
}

bool glfw3_layer::user_input::key_up(unsigned char c) const
{
	return _keys_up[c];
}

bool glfw3_layer::user_input::any_of_key(char const * s) const
{
	while (*s)
		if (key(*s++))
			return true;
	return false;
}

bool glfw3_layer::user_input::any_of_key_up(char const * s) const
{
	while (*s)
		if (key_up(*s++))
			return true;
	return false;
}

bool glfw3_layer::user_input::mouse(button b) const
{
	return _mouse_buttons[(int)b];
}

bool glfw3_layer::user_input::mouse_up(button b) const
{
	return _mouse_buttons_up[(int)b];
}

bool glfw3_layer::user_input::mouse_wheel(wheel w) const
{
	return _mouse_buttons_up[w == wheel::up ? (int)button::wheel_up : (int)button::wheel_down];
}

ivec2 const & glfw3_layer::user_input::mouse_position() const
{
	return _mouse_pos;
}

void glfw3_layer::user_input::update()
{
	for (bool & k : _keys_up)  // set all keys up to false
		k = false;

	for (bool & b : _mouse_buttons_up)  // set all buttons up to false
		b = false;
}

void glfw3_layer::user_input::mouse_motion(int x, int y)
{
	_mouse_pos = glm::ivec2{x,y};
}

void glfw3_layer::user_input::mouse_passive_motion(int x, int y)
{
	_mouse_pos = glm::ivec2{x,y};
}

void glfw3_layer::user_input::mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y)
{
	_mouse_pos = glm::ivec2{x,y};

	if (s == event_handler::state::down)
		_mouse_buttons[(int)b] = true;
	else
	{
		_mouse_buttons[(int)b] = false;
		_mouse_buttons_up[(int)b] = true;
	}
}

void glfw3_layer::user_input::mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y)
{
	using eh = event_handler;

	if (w == eh::wheel::up)
		_mouse_buttons_up[(int)eh::button::wheel_up] = true;

	if (w == eh::wheel::down)
		_mouse_buttons_up[(int)eh::button::wheel_down] = true;
}

void glfw3_layer::user_input::key_typed(unsigned char c, event_handler::modifier m, int x, int y)
{
	std::clog << "key_typed(c=" << c << ")" << std::endl;

	_keys[c] = true;
}

void glfw3_layer::user_input::key_released(unsigned char c, event_handler::modifier m, int x, int y)
{
	_keys[c] = false;
	_keys_up[c] = true;
}

void glfw3_layer::user_input::touch_performed(int x, int y, int finger_id, event_handler::action a)
{}


namespace glfw_detail {

void keyboard_func(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (key > 255)
		return;

	event_handler::modifier m = to_modifier(mods);
	if (action == GLFW_PRESS)
		active_window()->key_typed((unsigned char)key, m, 0, 0);
	else
		active_window()->key_released((unsigned char)key, m, 0, 0);
}

event_handler::modifier to_modifier(int m)
{
	using eh = event_handler;

	int mods = eh::modifier::none;
	if (m & GLFW_MOD_SHIFT)
		mods |= eh::modifier::shift;
	if (m & GLFW_MOD_CONTROL)
		mods |= eh::modifier::ctrl;
	if (m & GLFW_MOD_ALT)
		mods |= eh::modifier::alt;

	return eh::modifier(mods);
}

glfw3_layer * active_window()
{
	return __window;
}

}  // glfw_detail

}  // ui

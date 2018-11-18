#include "glfw3_window.hpp"
#include <GLFW/glfw3.h>

namespace ui {


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

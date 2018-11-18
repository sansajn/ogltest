#pragma once
#include "gl/window.hpp"

namespace ui {

class glfw3_layer : public window_layer
{
public:
	using parameters = window_layer::parameters;

	glfw3_layer(parameters const & params);
	~glfw3_layer() override;
	void display() override;
	void reshape(int w, int h) override;
	void main_loop_event() override;

	class user_input
	{
	public:
		void update() {}  //!< for internal use only

		// povinne rozhranie
		void mouse_motion(int x, int y) {}
		void mouse_passive_motion(int x, int y) {}
		void mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y) {}
		void mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y) {}
		void key_typed(unsigned char c, event_handler::modifier m, int x, int y) {}
		void key_released(unsigned char c, event_handler::modifier m, int x, int y) {}
		void touch_performed(int x, int y, int finger_id, event_handler::action a) {}
	};
};

using glfw_pool_window = ui::window<ui::pool_behaviour, ui::glfw3_layer>;
using glfw_window = glfw_pool_window;

}  // ui

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
		user_input();

		// verejne rozhranie
		bool key(unsigned char c) const;
		bool key_up(unsigned char c) const;
		bool any_of_key(char const * s) const;
		bool any_of_key_up(char const * s) const;
		bool mouse(button b) const;
		bool mouse_up(button b) const;
		bool mouse_wheel(wheel w) const;
		glm::ivec2 const & mouse_position() const;
		// TODO: special keys support

		void update();  //!< for internal use only

		// povinne rozhranie
		void mouse_motion(int x, int y);
		void mouse_passive_motion(int x, int y);
		void mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y);
		void mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y);
		void key_typed(unsigned char c, event_handler::modifier m, int x, int y);
		void key_released(unsigned char c, event_handler::modifier m, int x, int y);
		void touch_performed(int x, int y, int finger_id, event_handler::action a);

	private:
		static unsigned const NUM_KEYS = 256;
		static unsigned const NUM_BUTTONS = (unsigned)button::number_of_buttons;

		bool _keys[NUM_KEYS];
		bool _keys_up[NUM_KEYS];
		bool _mouse_buttons[NUM_BUTTONS];
		bool _mouse_buttons_up[NUM_BUTTONS];
		glm::ivec2 _mouse_pos;
	};
};

using glfw_pool_window = ui::window<ui::pool_behaviour, ui::glfw3_layer>;
using glfw_window = glfw_pool_window;

}  // ui

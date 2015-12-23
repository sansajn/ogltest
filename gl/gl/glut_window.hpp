#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/window.hpp"

namespace ui {

//! implmentacia okennej vrstvy v kniznici glut
class glut_layer : public window_layer  // TODO: layer sa tu nehodi, zmen to
{
public:
	using parameters = window_layer::parameters;

	glut_layer(parameters const & params);
	~glut_layer();

	void display() override {swap_buffers();}
	void install_display_handler() override;
	void main_loop() override {glutMainLoop();}
	void main_loop_event() override {glutMainLoopEvent();}
	void swap_buffers() override {glutSwapBuffers();}
	int modifiers() override {return glutGetModifiers();}

	struct user_input  //!< keyboard and mouse input
	{
		user_input();

		bool key(unsigned char c) const;
		bool key_up(unsigned char c) const;
		bool any_of_key(char const * s) const;
		bool any_of_key_up(char const * s) const;
		bool mouse(button b) const;
		bool mouse_up(button b) const;
		bool mouse_wheel(wheel w) const;
		glm::ivec2 const & mouse_position() const;
		// TODO: special support

		void update();  //!< for internal use only

	//private:

		static unsigned const NUM_KEYS = 256;
		static unsigned const NUM_BUTTONS = (unsigned)button::number_of_buttons;

		bool _keys[NUM_KEYS];
		bool _keys_up[NUM_KEYS];
		bool _mouse_buttons[NUM_BUTTONS];
		bool _mouse_buttons_up[NUM_BUTTONS];
		glm::ivec2 _mouse_pos;
	};
};  // glut_layer


/*! Glut based pool window.
\code
class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;
};
\endcode */
using glut_pool_window = window<pool_behaviour, glut_layer>;

using glut_event_window = window<event_behaviour, glut_layer>;
using glut_window = glut_pool_window;

}  // ui

#pragma once
#include <glm/vec2.hpp>
#include "window.hpp"
#include "camera.hpp"

namespace ui {

//! Implementacia okna vhodneho pre first-shoot-person scenu.
class fps_window : public glut_window
{
public:
	using base = glut_window;

	fps_window();
	virtual ~fps_window() {}

	void start() override;
	virtual void update(float dt) {}
	virtual void input() {}

	void close() override;

	glm::ivec2 center() const {return glm::ivec2(width()/2, height()/2);}

	// input (pull methods) TODO: presun do input struktury
	bool key(unsigned char c) const;
	bool key_up(unsigned char c) const;
	glm::ivec2 const & mouse_position() const {return _mouse_pos;}
	bool mouse(button b) const;
	bool mouse_up(button b) const;

private:
	void mouse_passive_motion(int x, int y) override;
	void mouse_click(button b, state s, modifier m, int x, int y) override;
	void key_typed(unsigned char c, modifier m, int x, int y) override;
	void key_released(unsigned char c, modifier m, int x, int y) override;

	void keyb_init();
	void mouse_init();
	void keyb_update();
	void mouse_update();

	bool _closed = false;

	// input
	static unsigned const NUM_KEYS = 256;
	bool _keys[NUM_KEYS];
	bool _keys_up[NUM_KEYS];
	glm::ivec2 _mouse_pos;
	bool _mouse_buttons[int(button::number_of_buttons)];
	bool _mouse_buttons_up[int(button::number_of_buttons)];
};

}  // ui

class camera_controller
{
public:
	virtual void input() = 0;
	virtual ~camera_controller() {}
};

class free_move : public camera_controller
{
public:
	free_move(camera & c, ui::fps_window & w, float movement = 0.1f) : _cam(c), _wnd(w), _movement(movement) {}
	void input() override;

	void movement(float v) {_movement = v;}

private:
	camera & _cam;
	ui::fps_window & _wnd;
	float _movement;
};

class free_look : public camera_controller
{
public:
	free_look(camera & c, ui::fps_window & w) : _cam(c), _wnd(w) {}
	void input() override;

private:
	camera & _cam;
	ui::fps_window & _wnd;
	bool _enabled = false;
};


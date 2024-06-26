#pragma once
#include <string>
#include <stdexcept>
#include <SDL.h>
#include <glm/glm.hpp>
#include "ui/window.hpp"

struct sdl_error : public window_error
{
	sdl_error(std::string const & s);
	std::string append_error_description(std::string const & s) const;
};

/*! @ingroup ui */
class sdl_window
	: public window
{
public:
	sdl_window(parameters const & params);
	~sdl_window();
	
	void start() override;
	void display(double t, double dt) override;

private:
	void handle_events();
	void handle_keyboard_event(SDL_KeyboardEvent const & e);
	void handle_mouse_button_event(SDL_MouseButtonEvent const & e);
	void handle_mouse_motion_event(SDL_MouseMotionEvent const & e);
	void handle_mouse_wheel_event(SDL_MouseWheelEvent const & e);

	double _t;
	double _dt;

	SDL_Window * _wnd;
	SDL_GLContext _glcontext;
	bool _quit;
};

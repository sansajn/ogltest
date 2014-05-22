#include "sdl_window.h"

namespace gl {

bool isspecial(SDL_Keysym const & k);
window::key tospecial(SDL_Keycode const k);
window::button tobutton(Uint8 btn);


sdl_window::sdl_window(parameters const & params)
	: _wnd(nullptr), _glcontext(nullptr), _quit(false)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw sdl_error("can't initialize SDL");

	// todo: tu chcem urcit verziu ogl kontextu

	_wnd = SDL_CreateWindow(params.name().c_str(), SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, params.width(), params.height(), SDL_WINDOW_OPENGL);
	if (!_wnd)
		throw sdl_error("can't create SDL window");

	_glcontext = SDL_GL_CreateContext(_wnd);
}

sdl_window::~sdl_window()
{
	SDL_GL_DeleteContext(_glcontext);
	SDL_DestroyWindow(_wnd);
	SDL_Quit();
}

void sdl_window::start()
{
	while (!_quit)
	{
		handle_events();
		if (_quit)
			return;
		display();
		idle();
	}
}

void sdl_window::display()
{
	SDL_GL_SwapWindow(_wnd);
}

void sdl_window::handle_events()
{
	SDL_Event event = {};
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{				
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				handle_keyboard_event(event.key);
				return;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				handle_mouse_button_event(event.button);
				return;

			case SDL_MOUSEMOTION:
				handle_mouse_motion_event(event.motion);
				return;

			case SDL_MOUSEWHEEL:
				handle_mouse_wheel_event(event.wheel);
				return;

			case SDL_QUIT:
				_quit = true;
				return;				
		}
	}  // while
}

void sdl_window::handle_keyboard_event(SDL_KeyboardEvent const & e)
{
	if (!isspecial(e.keysym))
	{
		unsigned char k = (unsigned char)e.keysym.sym;
		if (e.type == SDL_KEYDOWN)
			key_typed(k, modifier::alt, -1, -1);
		else
			key_released(k, modifier::alt, -1, -1);
	}
	else
	{
		key k = tospecial(e.keysym.sym);
		if (k == key::unknown)
			return;  // eat-unknown-keys

		if (e.type == SDL_KEYDOWN)
			special_key(k, modifier::alt, -1, -1);
		else
			special_key_released(k, modifier::alt, -1, -1);
	}
}

void sdl_window::handle_mouse_button_event(SDL_MouseButtonEvent const & e)
{
	mouse_click(tobutton(e.button),
		e.type == SDL_MOUSEBUTTONDOWN ? state::down : state::up,	modifier::alt, e.x, e.y);
}

void sdl_window::handle_mouse_motion_event(SDL_MouseMotionEvent const & e)
{
	if (e.state == 0)
		mouse_passive_motion(e.x, e.y);
	else
		mouse_motion(e.x, e.y);
}

void sdl_window::handle_mouse_wheel_event(SDL_MouseWheelEvent const & e)
{
	mouse_wheel(e.y > 0 ? wheel::up : wheel::down, modifier::alt, -1, -1);
}

window::button tobutton(Uint8 btn)
{
	return window::button(btn-1);
}

bool isspecial(SDL_Keysym const & k)
{
	return k.sym > SDLK_DELETE;
}

window::key tospecial(SDL_Keycode const k)
{
	if ((k > SDLK_CAPSLOCK-1) && (k < SDLK_UP+1))
		return window::key(k-SDLK_CAPSLOCK);
	else
		return window::key::unknown;
}

};  // gl

#include "sdl_window.h"

namespace gl {

bool isspecial(SDL_Keysym const & k);
window::key tospecial(SDL_Keycode const k);
window::button tobutton(Uint8 btn);


sdl_error::sdl_error(std::string const & s)
	: window_error(append_error_description(s))
{}

std::string sdl_error::append_error_description(std::string const & s) const
{
	std::string error = SDL_GetError();
	if (!error.empty())
		return s + " (description: " + error + ")";
	else
		return s;
}


sdl_window::sdl_window(parameters const & params)
	: _wnd(nullptr), _glcontext(nullptr), _quit(false)
{
	if (SDL_Init(SDL_INIT_VIDEO))
		throw sdl_error("can't initialize SDL library");

	if (params.version() != std::make_pair(-1, -1))
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.version().first);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.version().second);
	}

	_wnd = SDL_CreateWindow(params.name().c_str(), SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, params.size().first, params.size().second, SDL_WINDOW_OPENGL);
	if (!_wnd)
		throw sdl_error("can't create SDL window");

	_glcontext = SDL_GL_CreateContext(_wnd);
	if (!_glcontext)
		throw sdl_error("can't create GL context");

	window::glew_init();
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
			key_typed(k, -1, -1);
		else
			key_released(k, -1, -1);
	}
	else
	{
		key k = tospecial(e.keysym.sym);
		if (k == key::unknown)
			return;  // eat-unknown-keys

		if (e.type == SDL_KEYDOWN)
			special_key(k, -1, -1);
		else
			special_key_released(k, -1, -1);
	}
}

void sdl_window::handle_mouse_button_event(SDL_MouseButtonEvent const & e)
{
	mouse_click(tobutton(e.button),
		e.type == SDL_MOUSEBUTTONDOWN ? state::down : state::up,	e.x, e.y);
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
	mouse_wheel(e.y > 0 ? wheel::up : wheel::down, -1, -1);
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

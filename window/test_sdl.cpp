#include <iostream>
#include <GL/glew.h>
#include "sdl_window.h"

using std::cout;

using namespace gl;

class app_window
	: public sdl_window
{
public:
	app_window()
		: sdl_window(window::parameters().size(800, 600).name("SDL2 window"))
	{
		glClearColor(.5f, .5f, .5f, 1.0f);
	}

	void display()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		sdl_window::display();
	}

	void mouse_motion(int x, int y)
	{
		cout << "mouse_motion(x:" << x << ", y:" << y << ")\n";
		sdl_window::mouse_motion(x, y);
	}

	void mouse_passive_motion(int x, int y)
	{
		cout << "mouse_passive_motion(x:" << x << ", y:" << y << ")\n";
		sdl_window::mouse_passive_motion(x, y);
	}

	void mouse_wheel(wheel b, int x, int y)
	{
		cout << "mouse_wheel(b:" << (b == wheel::up ? "up" : "down") << ")\n";
		sdl_window::mouse_wheel(b, x, y);
	}

	void mouse_click(button b, state s, int x, int y)
	{
		cout << "mouse_click(b:" << button_name(b)
			<< ", s:" << (s == state::down ? "down" : "up")
			<< ", x:" << x << ", y:" << y << ")\n";
		sdl_window::mouse_click(b, s, x, y);
	}

	void key_typed(unsigned char c, int x, int y)
	{
		cout << "key_typed(c:" << c << ")\n";
		sdl_window::key_typed(c, x, y);
	}

	void key_released(unsigned char c, int x, int y)
	{
		cout << "key_released(c:" << c << ")\n";
		sdl_window::key_released(c, x, y);
	}

	void special_key(key k, int x, int y)
	{
		cout << "special_key(k:" << key_name(k) << ")\n";
		sdl_window::special_key(k, x, y);
	}

	void special_key_released(key k, int x, int y)
	{
		cout << "special_key_released(k:" << key_name(k) << ")\n";
		sdl_window::special_key_released(k, x, y);
	}
};


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

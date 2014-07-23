#include <iostream>
#include <GL/glew.h>
#include "ui/glut_window.h"

using std::cout;
using namespace gl;


class app_window
	: public glut_window
{
public:
	app_window()
		: glut_window(parameters().size(800, 600).version(1, 3).debug(true))
	{
		glClearColor(.5f, .5f, .5f, 1.0f);
	}

	void display()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glut_window::display();
	}

	void mouse_click(button b, state s, int x, int y)
	{
		cout << "mouse_click(b:" << button_name(b)
			<< ", s:" << (s == state::up ? "up" : "down") << ")\n";
		glut_window::mouse_click(b, s, x, y);
	}

	void mouse_motion(int x, int y)
	{
		cout << "mouse_motion(x:" << x << ", y:" << y << ")\n";
		glut_window::mouse_motion(x, y);
	}

	void mouse_passive_motion(int x, int y)
	{
		cout << "mouse_passive_motion(x:" << x << ", y:" << y << ")\n";
		glut_window::mouse_passive_motion(x, y);
	}

	void mouse_wheel(wheel b, int x, int y)
	{
		cout << "mouse_wheel(b:" << (b == wheel::up ? "up" : "down")
			<< ", x:" << x << ", y:" << y << ")\n";
		glut_window::mouse_wheel(b, x, y);
	}

	void key_typed(unsigned char c, int x, int y)
	{
		cout << "key_typed(c:" << c << ")\n";
		glut_window::key_typed(c, x, y);
	}

	void key_released(unsigned char c, int x, int y)
	{
		cout << "key_released(c:" << c << ")\n";
		glut_window::key_released(c, x, y);
	}

	void special_key(key k, int x, int y)
	{
		cout << "special_key(k:" << key_name(k) << ")\n";
		glut_window::special_key(k, x, y);
	}

	void special_key_released(key k, int x, int y)
	{
		cout << "special_key_released(k:" << key_name(k) << ")\n";
		glut_window::special_key_released(k, x, y);
	}
};


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

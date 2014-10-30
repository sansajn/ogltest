#include <iostream>
#include <GL/glew.h>
#include "ui/glut_window.hpp"

using std::cout;

class app_window : public glut_window
{
public:
	typedef glut_window base;

	app_window()
		: base(parameters().size(800, 600).version(1, 3).debug(true))
	{
		glClearColor(.5f, .5f, .5f, 1.0f);
	}

	void display(double t, double dt) override
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		base::display(t, dt);
	}

	bool mouse_click(button b, state s, modifier m, int x, int y) override
	{
		cout << "mouse_click(b:" << button_name(b)
			<< ", s:" << (s == state::up ? "up" : "down") << ")\n";
		return base::mouse_click(b, s, m, x, y);
	}

	bool mouse_motion(int x, int y) override
	{
		cout << "mouse_motion(x:" << x << ", y:" << y << ")\n";
		return base::mouse_motion(x, y);
	}

	bool mouse_passive_motion(int x, int y) override
	{
		cout << "mouse_passive_motion(x:" << x << ", y:" << y << ")\n";
		return base::mouse_passive_motion(x, y);
	}

	bool mouse_wheel(wheel b, modifier m, int x, int y) override
	{
		cout << "mouse_wheel(b:" << (b == wheel::up ? "up" : "down")
			<< ", x:" << x << ", y:" << y << ")\n";
		return base::mouse_wheel(b, m, x, y);
	}

	bool key_typed(unsigned char c, modifier m, int x, int y) override
	{
		cout << "key_typed(c:" << c << ")\n";
		return base::key_typed(c, m, x, y);
	}

	bool key_released(unsigned char c, modifier m, int x, int y) override
	{
		cout << "key_released(c:" << c << ")\n";
		return base::key_released(c, m, x, y);
	}

	bool special_key(key k, modifier m, int x, int y) override
	{
		cout << "special_key(k:" << key_name(k) << ")\n";
		return base::special_key(k, m, x, y);
	}

	bool special_key_released(key k, modifier m, int x, int y) override
	{
		cout << "special_key_released(k:" << key_name(k) << ")\n";
		return base::special_key_released(k, m, x, y);
	}
};


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

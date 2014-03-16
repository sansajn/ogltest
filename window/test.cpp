#include "glut_window.h"
#include <GL/glew.h>

using namespace gl;


class app_window
	: public glut_window
{
public:
	app_window()
		: glut_window(glut_window::parameters().size(800, 600).debug(true))
	{
		glClearColor(.5f, .5f, .5f, 1.0f);
	}

	void display()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glut_window::display();
	}
};


int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

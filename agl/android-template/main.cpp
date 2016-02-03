// android scene template
#include <iostream>
#include <cassert>
#include <jni.h>
#include <GL/glew.h>
#include "androidgl/android_window.hpp"

class scene_window : public ui::android_window
{
public:
	using base = ui::android_window;
	scene_window(parameters const & params);
	void display() override;
};

scene_window::scene_window(parameters const & params)
	: base{params}
{
	std::cout << "welcome in scene window ..." << std::endl;
	glClearColor(1,0,0,1);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	base::display();
}


scene_window * __w = nullptr;


void create(int width, int height)
{
	assert(!__w && "window alredy created");
	__w = new scene_window{scene_window::parameters{}.geometry(width, height)};
}

void destroy()
{
	delete __w;
	__w = nullptr;
}

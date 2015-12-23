#include "gl/glut_window.hpp"
#include "image.hpp"

using base_window = ui::glut_event_window;

class main_window : public base_window
{
public:
	main_window();
	void display() override;

private:
	ui::image _im;
};

main_window::main_window() : _im("assets/textures/lena.png")
{}

void main_window::display()
{
	_im.render();
	base_window::display();
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

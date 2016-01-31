// test touch tlacitka
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
#include "gl/glut_window.hpp"
#include "gl/ui/touch_button.hpp"
#include "gl/default_shaders.hpp"
#include "gl/resource.hpp"

using std::shared_ptr;


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void display() override;
	void input(float dt) override;
	void update(float dt) override;
	void close() override;

private:
	ui::touch::button _btn;
};

scene_window::scene_window()
{
	_btn.init(glm::ivec2{100, height() - 100}, 25, width(), height(),
		shader::default_shaders().get(shader::default_program_id::flat_shader_source));
}

void scene_window::display()
{
	_btn.render();
	base::display();
}

void scene_window::input(float dt)
{
	base::input(dt);

	// simulacia touch-eventu pomocou mysi
	if (in.mouse(button::left))
		_btn.touch(glm::ivec2{in.mouse_position()}, ui::touch::touch_event::down);
	if (in.mouse_up(button::left))
		_btn.touch(glm::ivec2{in.mouse_position()}, ui::touch::touch_event::up);

	if (_btn.down())
		std::cout << "button down" << std::endl;
	else if (_btn.up())
		std::cout << "button up/released" << std::endl;
}

void scene_window::update(float dt)
{
	_btn.update();
	base::update(dt);
}

void scene_window::close()
{
	resource_manager_list::ref().free();  // uvolni vsetky lokovane zdroje
	base::close();
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

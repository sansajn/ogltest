// renderuje steny, strop a dlazku levelu
#include <vector>
#include <string>
#include <glm/gtx/transform.hpp>
#include "window.hpp"
#include "scene_object.hpp"
#include "level.hpp"
#include "player.hpp"

using std::vector;
using std::string;
using glm::vec3;
using glm::radians;
using glm::translate;
using ui::glut_pool_window;

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	level _lvl;
	fps_player _player;
	axis_object _axis;
	light_object _light;
};

scene_window::scene_window()
	: base{parameters{}.name("test_level")}
{
	_player.init(_lvl.player_position(), radians(70.0), aspect_ratio(), 0.01, 1000, this);
	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_lvl.render(_player.get_camera());
	_axis.render(_player.get_camera().view_projection());
	_light.render(_player.get_camera().view_projection() * translate(light_pos));

	base::display();
}

void scene_window::input(float dt)
{
	_player.input(dt);  // pohyb levelom nefunguje kvoky fyzike
	base::input(dt);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_player.update(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

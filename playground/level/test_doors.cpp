// do levelu pridava dvere
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "window.hpp"
#include "program.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "level.hpp"
#include "player.hpp"
#include "physics/physics.hpp"
#include "physics/physics_debug.hpp"

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::translate;
using gl::camera;
using gl::free_camera;
using ui::glut_pool_window;


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	~scene_window() {}

	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	rigid_body_world _world;
	debug_drawer _ddraw;
	level _lvl;
	fps_player _player;
	axis_object _axis;
	light_object _light;

	free_camera<scene_window> _free_view;
	bool _player_view = false;
};

scene_window::scene_window()
	: base{parameters{}.name("level with wall collisions")}, _free_view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_world.debug_drawer(&_ddraw);

	_lvl.link_with(_world);

	_player.init(_lvl.player_position(), radians(70.0f), aspect_ratio(), 0.01, 1000, this);
	_player.link_with(_world);

	glClearColor(0, 0, 0, 1);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_world.update(dt);

	_lvl.update(dt);
	_player.update(dt);

	camera * cam = _player_view ? &_player.get_camera() : &_free_view.get_camera();
	_ddraw.update(cam->view_projection());
}

void scene_window::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera * cam = &_free_view.get_camera();
	if (_player_view)
		cam = &_player.get_camera();

	_lvl.render(*cam);
	_axis.render(cam->view_projection());
	_light.render(cam->view_projection() * translate(light_pos));

	_world.debug_draw();

	base::display();
}

void scene_window::input(float dt)
{
	if (_player_view)
		_player.input(dt);
	else
		_free_view.input(dt);

	if (in.key(' '))  // open door
	{
		door * d = _lvl.find_door(_player.body()->getWorldTransform(), _world);
		if (d)
			d->open();
	}

	if (in.key_up('g'))
		_ddraw.toggle_debug_flag(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);

	if (in.key_up('1'))
		_player_view = false;

	if (in.key_up('2'))
		_player_view = true;

	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

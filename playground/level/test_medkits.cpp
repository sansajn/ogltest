// do levelu pridava dvere
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "window.hpp"
#include "program.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "medkit_world.hpp"
#include "level.hpp"
#include "sound.hpp"

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

using namespace phys;

string health_sound_path = "assets/sound/health.ogg";
string door_sound_path = "assets/sound/door.ogg";
string level_music_path = "assets/sound/03_-_Wolfenstein_3D_-_DOS_-_Get_Them_Before_They_Get_You.ogg";

class medkit_pick_listenner : public collision_listener
{
public:
	medkit_pick_listenner(level & lvl, rigid_body_world & world) : _lvl{lvl}, _world{world} {}
	void collision_event(btCollisionObject * body0, btCollisionObject * body1) override;

	bool medkit_picked = false;

private:
	unsigned count = 0;
	level & _lvl;
	rigid_body_world & _world;
};

void medkit_pick_listenner::collision_event(btCollisionObject * body0, btCollisionObject * body1)
{
	btCollisionObject * medkit = nullptr, * other = nullptr;
	if (((btCollisionObject *)body0)->getUserIndex() == (int)medkit_world::collision_object_type::medkit)
	{
		medkit = (btCollisionObject *)body0;
		other = (btCollisionObject *)body1;
	}
	else if (((btCollisionObject *)body1)->getUserIndex() == (int)medkit_world::collision_object_type::medkit)
	{
		medkit = (btCollisionObject *)body1;
		other = (btCollisionObject *)body0;
	}

	if (medkit)
		std::cout << "medkit collision #" << ++count << std::endl;

	if (medkit && (other->getUserIndex() == (int)medkit_world::collision_object_type::player))
	{
		_world.native()->removeCollisionObject(medkit);  // vyber lekarnicku zo sveta
		_lvl.remove_medkit(medkit);
		medkit_picked = true;
	}
}


class medkit_scene : public glut_pool_window
{
public:
	using base = glut_pool_window;

	medkit_scene();
	~medkit_scene();

	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	medkit_world _world;
	level _lvl;
	medkit_pick_listenner _medkit_collision;
	fps_player _player;
	axis_object _axis;
	light_object _light;

	free_camera<medkit_scene> _free_view;
	bool _player_view = false;
};

medkit_scene::medkit_scene()
	: base{parameters{}.name("level with medkits")}
	, _medkit_collision{_lvl, _world}
	, _free_view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_world.add_collision_listener(&_medkit_collision);

	_lvl.link_with(_world);

	_player.init(_lvl.player_position(), radians(70.0f), aspect_ratio(), 0.01, 1000, this);
	_world.link(_player);

	al::default_device->play_music(level_music_path);

	glClearColor(0, 0, 0, 1);
}

medkit_scene::~medkit_scene()
{}

void medkit_scene::update(float dt)
{
	base::update(dt);
	_world.update(dt);
	_lvl.update(dt);
	_player.update(dt);

	if (_medkit_collision.medkit_picked)
	{
		al::default_device->play_effect(health_sound_path);
		_medkit_collision.medkit_picked = false;
	}
}

void medkit_scene::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera * cam = &_free_view.get_camera();
	if (_player_view)
		cam = &_player.get_camera();

	_lvl.render(*cam);

	auto VP = cam->view_projection();
	_axis.render(VP);
	_light.render(VP * translate(light_pos));
	_world.debug_render(VP);

	base::display();
}

void medkit_scene::input(float dt)
{
	if (_player_view)
		_player.input(dt);
	else
		_free_view.input(dt);

	if (in.key(' '))  // open door
	{
		door_object * d = _lvl.find_door(_player.body()->getWorldTransform(), _world);
		if (d)
			d->open();
	}

	if (in.key_up('1'))
		_player_view = false;

	if (in.key_up('2'))
		_player_view = true;

	base::input(dt);
}


int main(int argc, char * argv[])
{
	al::init_sound_system();

	medkit_scene w;
	w.start();

	al::free_sound_system();
	return 0;
}

// do levelu pridava dvere
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gl/program.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
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

string const health_sound_path = "assets/sound/health.ogg";
string const door_sound_path = "assets/sound/door.ogg";
string const level_music_path = "assets/sound/03_-_Wolfenstein_3D_-_DOS_-_Get_Them_Before_They_Get_You.ogg";

string const skinned_shader_path = "assets/shaders/bump_skinned.glsl";


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


class w3dclone_scene : public glut_pool_window
{
public:
	using base = glut_pool_window;

	w3dclone_scene();
	~w3dclone_scene();

	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	medkit_world _world;  // fyzika
	level _lvl;
	medkit_pick_listenner _medkit_collision;
	player_object _player;
	axis_object _axis;
	light_object _light;
	shader::program _player_prog;

	free_camera<w3dclone_scene> _free_view;
	bool _player_view = true;
};

w3dclone_scene::w3dclone_scene()
	: base{parameters{}.name("level with medkits")}
	, _medkit_collision{_lvl, _world}
	, _free_view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_world.add_collision_listener(&_medkit_collision);

	_lvl.link_with(_world);

	_player.init(_lvl.player_position(), radians(70.0f), aspect_ratio(), 0.01, 1000, this);
	_world.link(_player);

	_player_prog.from_file(skinned_shader_path);

	// vytvor herny svet
	game_world & game = game_world::ref();
	game._player = &_player;

//	al::default_device->play_music(level_music_path);  // pusti podmaz TODO: tu chcem loop

	glClearColor(0, 0, 0, 1);
}

w3dclone_scene::~w3dclone_scene()
{}

void w3dclone_scene::update(float dt)
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

void w3dclone_scene::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera * cam = &_free_view.get_camera();
	if (_player_view)
		cam = &_player.get_camera();

	_lvl.render(*cam);

	if (_player_view)
	{
		camera * cam = &_player.get_camera();
		// player
		mat4 const & V = cam->view();
		mat4 M = mat4{1};
		M *= inverse(V);
		M = rotate(M, radians(90.0f), vec3{0, 1, 0});
		M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
		mat4 world_to_camera = V;
		mat4 local_to_screen = cam->projection() * world_to_camera * M;
		mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera * M)};

		_player_prog.use();
		_player_prog.uniform_variable("local_to_world", M);
		_player_prog.uniform_variable("world_to_camera", world_to_camera);
		_player_prog.uniform_variable("local_to_screen", local_to_screen);
		_player_prog.uniform_variable("normal_to_camera", normal_to_camera);
		_player_prog.uniform_variable("light.direction", normalize(light_pos));
		_player_prog.uniform_variable("skeleton", _player.skeleton());

//		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		_player.render(_player_prog);
	}

	auto VP = cam->view_projection();
//	_axis.render(VP);
//	_light.render(VP * translate(light_pos));
//	_world.debug_render(VP);

	base::display();
}

void w3dclone_scene::input(float dt)
{
	if (_player_view)
		_player.input(dt);
	else
		_free_view.input(dt);

	if (in.key('e'))  // open door
	{
		door_object * d = _lvl.find_door(_player.body()->getWorldTransform(), _world);
		if (d)
			d->open();
	}

	if (in.key(' '))  // shoot
	{
		_player.fire();
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
	texman.root_path("assets/blaster");

	w3dclone_scene w;
	w.start();

	al::free_sound_system();
	return 0;
}

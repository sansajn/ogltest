// do levelu pridava kolizie
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include "window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "level.hpp"
#include "player.hpp"

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using gl::mesh;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	~scene_window();

	// (?) naco su tieto funkcie public ?
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	// collisions
	void init_physics();
	void shutdown_physics();
	void check_for_collision_events();
	void collision_event(btRigidBody * body0, btRigidBody * body1);
	void separation_event(btRigidBody * body0, btRigidBody * body1);

	level _lvl;
	player _player;
	axis_object _axis;
	light_object _light;

	btDynamicsWorld * _world;
	btBroadphaseInterface * _broadhpase;
	btCollisionConfiguration * _collision_configuration;
	btCollisionDispatcher * _dispatcher;
	btConstraintSolver * _solver;
};  // scene_window

void scene_window::init_physics()
{
	_collision_configuration = new btDefaultCollisionConfiguration{};
	_dispatcher = new btCollisionDispatcher{_collision_configuration};
	_broadhpase = new btDbvtBroadphase{};
	_solver = new btSequentialImpulseConstraintSolver{};
	_world = new btDiscreteDynamicsWorld{_dispatcher, _broadhpase, _solver, _collision_configuration};
}

void scene_window::shutdown_physics()
{
	delete _world;
	delete _solver;
	delete _broadhpase;
	delete _dispatcher;
	delete _collision_configuration;
}

void scene_window::update(float dt)
{
	if (_world)
	{
		_world->stepSimulation(dt);
		check_for_collision_events();
	}

	base::update(dt);
}

void scene_window::check_for_collision_events()
{}

void scene_window::collision_event(btRigidBody * body0, btRigidBody * body1)
{
	// jedno s tiel je player narazajuci do steny ...
	_player.position(_player.prev_position());  // staci playera presunut na predchadzajucu poziciu ...
}

void scene_window::separation_event(btRigidBody * body0, btRigidBody * body1)
{}

scene_window::scene_window()
	: _player{*this}
	, _world{nullptr}
	, _broadhpase{nullptr}
	, _collision_configuration{nullptr}
	, _dispatcher{nullptr}
	, _solver{nullptr}
{
	// TODO: treba volat v spravnom poradi
	view v;
	v.fovy = radians(70.0);
	v.aspect_ratio = aspect_ratio();
	v.near = 0.01;
	v.far = 1000.0;
	_player.view_parameters(v);
	_player.position(_lvl.player_position());

	init_physics();

	glClearColor(0, 0, 0, 1);
}

scene_window::~scene_window()
{
	shutdown_physics();
}

void scene_window::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_lvl.render(_player.get_camera());
	_axis.render(_player.get_camera());
	_light.render(_player.get_camera(), light_pos);

	base::display();
}

void scene_window::input(float dt)
{
	_player.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

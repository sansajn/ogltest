#include "player.hpp"
#include "geometry/utility.hpp"
#include "sound.hpp"
#include "enemy.hpp"
#include <iostream>

using std::vector;
using std::string;
using std::max;
using std::min;
using glm::ivec2;
using glm::vec3;
using glm::mat3;
using glm::quat;
using glm::mat3_cast;
using glm::radians;
using gl::camera;
using gl::skeletal_animation;
using geom::right;
using geom::forward;
using ui::glut_pool_window;

using namespace phys;

// quake 4 blaster
string const model_path = "assets/blaster/view.md5mesh";

string const anim_paths[] = {
	"assets/blaster/big_recoil.md5anim",
	"assets/blaster/charge_up.md5anim",
	"assets/blaster/fire.md5anim",
	"assets/blaster/fire2.md5anim",
	"assets/blaster/flashlight.md5anim",
	"assets/blaster/idle.md5anim",
	"assets/blaster/lower.md5anim",
	"assets/blaster/raise.md5anim"
};

string const effect_paths[] = {
	"assets/sound/fire01.ogg"
};


void fps_move::init(glut_pool_window::user_input * in, btRigidBody * body, float velocity)
{
	assert(in && body && "invalid pointer");
	_in = in;
	_body = body;
	_velocity = velocity;
	strcpy(_controls, "wsad");  // default controls
}

void fps_move::input(float dt)
{
	vec3 vel{0,0,0};

	if (_in->one_of_key(_controls))
	{
		mat3 r = mat3_cast(glm_cast(_body->getOrientation()));
		vec3 right_dir = right(r);
		vec3 forward_dir = forward(r);

		if (_in->key(_controls[(int)key::forward]))
			vel -= forward_dir;
		if (_in->key(_controls[(int)key::backward]))
			vel += forward_dir;
		if (_in->key(_controls[(int)key::left]))
			vel -= right_dir;
		if (_in->key(_controls[(int)key::right]))
			vel += right_dir;

		vel.y = 0;  // chcem sa pohybovat iba po rovine
		vel *= _velocity;
	}

	_body->setLinearVelocity(bullet_cast(vel));
}

void fps_look::init(glut_pool_window * window, btRigidBody * body, float velocity)
{
	_window = window;
	_body = body;
	_velocity = velocity;
}

void fps_look::input(float dt)
{
	ivec2 screen_center = _window->center();

	if (_window->in.mouse(glut_pool_window::button::left) && !_enabled)
	{
		_enabled = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutWarpPointer(screen_center.x, screen_center.y);
		return;
	}

	if (_window->in.key_up(27) && _enabled)  // esc
	{
		_enabled = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (!_enabled)
		return;

	float const angular_movement = 0.1f;
	ivec2 delta = _window->in.mouse_position() - screen_center;

	if (delta.x != 0)
	{
		float angle = radians(angular_movement * delta.x);
		btTransform transf = _body->getCenterOfMassTransform();
		btQuaternion r = btQuaternion{btVector3{0,1,0}, -angle} * transf.getRotation();
		_body->setCenterOfMassTransform(btTransform{r, transf.getOrigin()});
	}

	if (delta.y != 0)
	{
		float angle = radians(angular_movement * delta.y);
		btTransform transf = _body->getCenterOfMassTransform();
		btVector3 right_dir = transf.getBasis().getColumn(0);
		btQuaternion r = btQuaternion{right_dir, -angle} * transf.getRotation();
		_body->setCenterOfMassTransform(btTransform{r, transf.getOrigin()});
	}

	if (delta.x != 0 || delta.y != 0)
		glutWarpPointer(screen_center.x, screen_center.y);
}

void fps_player::init(vec3 const & position, float fovy, float aspect_ratio, float near, float far, glut_pool_window * window)
{
	_cam = camera{fovy, aspect_ratio, near, far};

	float const mass = 90.0f;
	_collision = body_object{make_box_shape(btVector3{0.25, 0.25, 0.25}), mass, btVector3{0, 0.5, 0} + bullet_cast(position)};
	_collision.native()->setAngularFactor(0);  // nechcem aby sa hrac otacal pri kolizii
	_collision.native()->setActivationState(DISABLE_DEACTIVATION);

	_window = window;
	_look.init(_window, _collision.native());
	_move.init(&_window->in, _collision.native());
}

void fps_player::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
}

void fps_player::link_with(rigid_body_world & world, int mark)
{
	world.link(_collision);
	_collision.native()->setGravity(btVector3{0,0,0});  // turn off object gravity
	if (mark != -1)
		_collision.native()->setUserIndex(mark);
}

void fps_player::update(float dt)
{
	_cam.position = glm_cast(_collision.position());
	_cam.rotation = glm_cast(_collision.rotation());
}

void player_object::init(glm::vec3 const & position, float fovy, float aspect_ratio, float near, float far, ui::glut_pool_window * window)
{
	_mdl = animated_textured_model_from_file(model_path);

	for (string const & anim_path : anim_paths)
		_mdl.append_animation(skeletal_animation{anim_path});

	_mdl.animation_sequence(vector<unsigned>{idle_animation});

	_cam = camera{fovy, aspect_ratio, near, far};

	float const mass = 90.0f;
	_collision = body_object{make_box_shape(btVector3{0.25, 0.25, 0.25}), mass, btVector3{0, 0.5, 0} + bullet_cast(position)};
	_collision.native()->setAngularFactor(0);  // nechcem aby sa hrac otacal pri kolizii
	_collision.native()->setActivationState(DISABLE_DEACTIVATION);

	_window = window;
	_look.init(_window, _collision.native());
	_move.init(&_window->in, _collision.native());
}

void player_object::link_with(rigid_body_world & world, int mark)
{
	world.link(_collision);
	_collision.native()->setGravity(btVector3{0,0,0});  // turn off object gravity
	if (mark != -1)
		_collision.native()->setUserIndex(mark);
}

void player_object::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
}

void player_object::update(float dt)
{
	_cam.position = glm_cast(_collision.position());
	_cam.rotation = glm_cast(_collision.rotation());
	_state.update(dt, this);
	_mdl.update(dt);
}

void player_object::render(shader::program & prog)
{
	_mdl.render(prog);
}

void player_object::fire()
{
	_state.enter_fire_sequence();
}

void player_object::damage(unsigned amount)
{
	_health = max(0u, _health - amount);
}

void player_object::heal(unsigned amount)
{
	_health = min(100u, _health + amount);
}

player_state_machine::player_state_machine() : state_machine{player_states::idle}
{
	fill_states();
}

void player_state_machine::enter_fire_sequence()
{
	if (current_state() == player_states::idle)
		enqueue_state(player_states::fire);
}

player_state_machine_state & player_state_machine::to_ref(state_descriptor s)
{
	return *_states[(int)s];
}

void player_state_machine::fill_states()
{
	_states[(int)player_states::idle] = &_idle;
	_states[(int)player_states::fire] = &_fire;
}

void player_idle::enter(player_object * p)
{
	p->get_model().animation_sequence(vector<unsigned>{player_object::idle_animation}, animated_textured_model::repeat_mode::loop);
}

player_states player_idle::update(float dt, player_object * p)
{
	return player_states::invalid;
}

inline btVector3 forward(btMatrix3x3 const & M)
{
	return M.getColumn(2);
}

void player_fire::enter(player_object * p)
{
	_t = 0;
	p->get_model().animation_sequence(vector<unsigned>{player_object::fire_animation}, animated_textured_model::repeat_mode::once);  // TODO: dynamicka alokacia (pomale), TODO: specializovana funkcia pre sekvencie dlzky 1
	al::default_device->play_effect(effect_paths[player_object::fire1_sfx]);

	// vystrel na ciel
	game_world & game = game_world::ref();
	phys::rigid_body_world * world = game.physics();
	btTransform const & player_transf = game.player()->transform();
	btVector3 from = player_transf.getOrigin();
	btVector3 to = from + (-100.0f * forward(player_transf.getBasis()));
	btCollisionWorld::ClosestRayResultCallback rayres{from, to};
	world->native()->rayTest(from, to, rayres);
	if (rayres.hasHit() && rayres.m_collisionObject->getUserIndex() == (int)game_world::object_types::enemy)  // trafil som nepriatela
	{
		enemy_object & e = game.get_enemy(rayres.m_collisionObject);
		e.damage(10);   // TODO: znahodni hodnotu poskodenia
		std::cout << "enemy id:" << e._id << " was shoot, health:" << e.health() << std::endl;
	}
}

player_states player_fire::update(float dt, player_object * p)
{
	_t += dt;
	if (_t >= DURATION)
		return player_states::idle;

	return player_states::invalid;
}


crosshair_object::crosshair_object()
{
	_quad = gl::make_quad_xy();
	_tex = texture2d{"assets/textures/crosshair_blaster.tga",
		texture::parameters().min(texture_filter::nearest).mag(texture_filter::nearest)};
}

void crosshair_object::render(shader::program & p, glm::mat4 const & local_to_screen)
{
	p.uniform_variable("local_to_screen", local_to_screen);
	_tex.bind(0);
	p.uniform_variable("s", 0);
	_quad.render();
}

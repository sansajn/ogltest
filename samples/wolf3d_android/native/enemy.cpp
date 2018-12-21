#include "enemy.hpp"
#include <algorithm>
#include <cmath>
#include "sound.hpp"
#include "player.hpp"
#include <iostream>  // TODO: remove

using std::string;
using std::max;
using std::shared_ptr;
using std::floor;
using glm::vec3;
using glm::mat4;

using namespace phys;

string const death1 = "/sdcard/wolf3/sound/death1.ogg";
string const death2 = "/sdcard/wolf3/sound/death2.ogg";
string const machine_gun = "/sdcard/wolf3/sound/machine_gun.ogg";

shared_ptr<btCollisionShape> enemy_object::_shape;

static int id_counter = 1;


enemy_object::enemy_object(btVector3 const & position)
{
	std::clog << "enemy_object::enemy_object()" << std::endl;

	_collision = body_object{shared_shape(), 90.0f, position + btVector3{.5, .45, -.5}};
	_collision.native()->setLinearFactor(btVector3{0,0,0});
	_collision.native()->setAngularFactor(0);

	string files[] = {
		"/sdcard/wolf3/textures/enemy.png",
		"/sdcard/wolf3/textures/sswva1.png",  // 1: run
		"/sdcard/wolf3/textures/sswvb1.png",
		"/sdcard/wolf3/textures/sswvc1.png",
		"/sdcard/wolf3/textures/sswvd1.png",
		"/sdcard/wolf3/textures/sswve0.png",  // 5: shooting
		"/sdcard/wolf3/textures/sswvf0.png",
		"/sdcard/wolf3/textures/sswvg0.png",
		"/sdcard/wolf3/textures/sswvh0.png",
		"/sdcard/wolf3/textures/sswvi0.png",  // 9: death
		"/sdcard/wolf3/textures/sswvj0.png",
		"/sdcard/wolf3/textures/sswvk0.png",
		"/sdcard/wolf3/textures/sswvl0.png",
		"/sdcard/wolf3/textures/sswvm0.png"  // 13
	};

	_model = sprite_model{files, sizeof(files)/sizeof(files[0]), 64, 64};
	_model.frame_rate(5);

	_id = id_counter++;
}

enemy_object::~enemy_object()
{
	if (_shape.use_count() == 1)
		_shape.reset();
}

btTransform const & enemy_object::transform() const
{
	return _collision.transform();
}

btCollisionObject * enemy_object::collision() const
{
	return _collision.native();
}

void enemy_object::update(float dt)
{
	auto const & play = game_world::ref().player()->transform();
	_rot = glm_cast(-play.getRotation());
	_model.update(dt);
	_state.update(dt, this);
}

void enemy_object::render(gles2::shader::program & p, glm::mat4 const & world_to_screen)
{
	mat4 T = translate(glm_cast(_collision.position() + btVector3{0, -.05, 0}));
	mat4 R = mat4_cast(_rot);
	mat4 M = T * R * scale(.4f * vec3{1, 1, 1});
	p.uniform_variable("local_to_screen", world_to_screen * M);
	_model.render(p);
}

void enemy_object::link_with(rigid_body_world & world, int mask)
{
	world.link(_collision);
	world.native()->setGravity(btVector3{0,0,0});
	if (mask != -1)
		_collision.native()->setUserIndex(mask);
}

void enemy_object::go(btVector3 const & velocity)
{
	btRigidBody * body = _collision.native();
	if (!body->isActive())
		body->activate();
	body->setLinearVelocity(velocity);
}

bool enemy_object::see_player() const
{
	game_world & game = game_world::ref();
	btVector3 const & player_pos = game.player()->transform().getOrigin();
	btVector3 const & enemy_pos = transform().getOrigin();
	phys::rigid_body_world * world = game.physics();
	btCollisionWorld::ClosestRayResultCallback rayres{enemy_pos, player_pos};
	world->native()->rayTest(enemy_pos, player_pos, rayres);
	return rayres.hasHit() && (rayres.m_collisionObject->getUserIndex() == (int)game_world::object_types::player);
}

float enemy_object::player_distance2() const
{
	btVector3 const & enemy_pos = transform().getOrigin();
	return enemy_pos.distance2(game_world::ref().player()->transform().getOrigin());
}

void enemy_object::damage(unsigned amount)
{
	_health = max(0u, _health - amount);
}

void enemy_object::remove_from_world()
{
	game_world & game = game_world::ref();
	game.physics()->unlink(_collision);
}

std::shared_ptr<btCollisionShape> enemy_object::shared_shape()
{
	if (!_shape)
		_shape = make_box_shape(btVector3{.2, .2, .2});
	return _shape;
}

void enemy_guard::enter(enemy_object * e)
{
	std::cout << "enemy_guard::enter(id:" << e->_id << ")" << std::endl;
}

inline btVector3 player_grid_position()
{
	btVector3 r = game_world::ref().player()->transform().getOrigin();
	return btVector3{floor(r.x())+.5f, r.y(), floor(r.z())-.5f};
}

enemy_states enemy_guard::update(float dt, enemy_object * e)
{
	// guard -> attention, death

	// death : zivot na 0
	if (e->health() == 0)
		return enemy_states::death;

	// attention : vidim hraca a hrac je blizko
	if (e->player_distance2() < 64.0f && e->see_player())  // <8
	{
		e->last_known_player_pos = player_grid_position();
		return enemy_states::attention;
	}

	// attention : pocujem strelbu TODO: implement

	return enemy_states::invalid;
}

void enemy_attention::enter(enemy_object * e)
{
	_t = 0;
	std::cout << "enemy_attention::enter(id:" << e->_id << ")" << std::endl;
}

enemy_states enemy_attention::update(float dt, enemy_object * e)
{
	// attention -> fight, death
	_t += dt;

	// death : zivot na 0
	if (e->health() == 0)
		return enemy_states::death;

	// fight : hrac je prilis blizko
	if (e->player_distance2() < 1.0f)
		return enemy_states::fight;

	// fight : striela sa na mna, TODO: implement

	// fight : po prekroceni casoveho intervalu
	if (_t > MAX_ATTENTION_TIME)
		return enemy_states::fight;

	return enemy_states::invalid;
}

void enemy_fight::enter(enemy_object * e)
{
	_t = 0;
	std::cout << "enemy_fight::enter(id:" << e->_id << ")" << std::endl;
}

enemy_states enemy_fight::update(float dt, enemy_object * e)
{
	// fight -> chase, death
	_t += dt;

	// death : zivot na 0
	if (e->health() == 0)
		return enemy_states::death;

	// chase : hrac je daleko
	if (e->player_distance2() > 49.0f)  // >7
		return enemy_states::chase;

	// chase : utek hraca (na hraca nie je priamy vyhlad)
	if (!e->see_player())
	{
		e->last_known_player_pos = player_grid_position();
		return enemy_states::chase;
	}

	// strielaj na hraca
	if (_t > _fire_period)
	{
		e->model().animation_sequence(5, 9);
		al::device::ref().play_effect(machine_gun);
		((player_object *)game_world::ref().player())->damage(10);
		_t = 0;
	}

	return enemy_states::invalid;
}

void enemy_chase::enter(enemy_object * e)
{
	_t = 0;
	e->model().animation_sequence(1, 5, sprite_model::repeat_mode::loop);
	std::cout << "enemy_chase::enter(id:" << e->_id << ")" << std::endl;
}

inline bool near(btVector3 const & a, btVector3 const & b)
{
	return a.distance2(b) < (.1f*.1f);
}

enemy_states enemy_chase::update(float dt, enemy_object * e)
{
	// chase -> fight, guard, death
	_t += dt;

	// death : zivot na 0
	if (e->health() == 0)
		return enemy_states::death;

	// fight : hrac na dohlad a blizko
	if (e->player_distance2() < 25.0f)  // <5
	{
		if (e->see_player())
			return enemy_states::fight;
	}

	// fight : presun sa na poslednu znamu poziciu hraca
	if (near(e->position(), e->last_known_player_pos))
	{
		return enemy_states::fight;
	}
	else
	{
		btVector3 dir = e->last_known_player_pos - e->position();
		dir.normalize();
		e->go(1.1f * dir);
	}

	// guard : hrac dostatocne dlho mimo dohladu
	if (_t > MAX_PLAYER_UNKNOWN_TIME)
		return enemy_states::guard;

	return enemy_states::invalid;
}

void enemy_chase::exit(enemy_object * e)
{
	e->go(btVector3{0,0,0});  // zastav postavu
}

void enemy_death::enter(enemy_object * e)
{
	game_world & game = game_world::ref();
	string const & death = game.rand() % 2 ? death1 : death2;
	al::device::ref().play_effect(death);
	e->model().animation_sequence(9, 14);
	e->remove_from_world();
}

enemy_states enemy_death::update(float dt, enemy_object * e)
{
	// death
	return enemy_states::invalid;
}

enemy_state_machine:: enemy_state_machine() : state_machine{enemy_states::guard}
{
	fill_states();
}

enemy_state_machine_state & enemy_state_machine::to_ref(state_descriptor s)
{
	return *_states[(int)s];
}

void enemy_state_machine::fill_states()
{
	_states[(int)enemy_states::guard] = &_guard;
	_states[(int)enemy_states::attention] = &_attention;
	_states[(int)enemy_states::fight] = &_fight;
	_states[(int)enemy_states::chase] = &_chase;
	_states[(int)enemy_states::death] = &_death;
}

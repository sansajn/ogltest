//! herny svet
#pragma once
#include <random>
#include "physics/physics.hpp"

class game_object
{
public:
	virtual ~game_object() {}
	virtual btTransform const & transform() const = 0;
	virtual btCollisionObject * collision() const {return nullptr;}
};

class enemy_object;

class game_world  // umoznuje interakiu hernych objektov
{
public:
	enum class object_types {player, medkit, door, enemy};

	static game_world & ref();

	std::default_random_engine rand;  //!< nahodne cele cisla

	game_object * player() const {return _player;}
	phys::rigid_body_world * physics() const {return _physics;}
	enemy_object & get_enemy(btCollisionObject const * obj);

	// TODO: zrejme iba docastne ;)
	game_object * _player = nullptr;
	phys::rigid_body_world * _physics = nullptr;
	std::vector<enemy_object *> * _enemies = nullptr;

private:
	game_world();
};

#pragma once
#include "door.hpp"
#include "medkit.hpp"
#include "player.hpp"
#include "enemy.hpp"

class medkit_world : public phys::rigid_body_world  // TODO: premenuj
{
public:
	using base = phys::rigid_body_world;

	enum class collision_object_type {player, medkit, door, enemy};

	using base::link;
	void link(player_object & p);
	void link(medkit_object & m);
	void link(door_object & d);
	void link(enemy_object & e);
};

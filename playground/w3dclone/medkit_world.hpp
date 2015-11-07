#pragma once
#include "door.hpp"
#include "medkit.hpp"
#include "player.hpp"

class medkit_world : public phys::rigid_body_world
{
public:
	using base = phys::rigid_body_world;
	enum class collision_object_type {player, medkit, door};

	using base::link;
//	void link(fps_player & p);
	void link(player_object & p);
	void link(medkit_object & m);
	void link(door_object & d);
};

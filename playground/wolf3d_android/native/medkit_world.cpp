#include "medkit_world.hpp"

void medkit_world::link(player_object & p)
{
	p.link_with(*this, (int)collision_object_type::player);
}

void medkit_world::link(medkit_object & m)
{
	m.link_with(*this, (int)collision_object_type::medkit);
}

void medkit_world::link(door_object & d)
{
	d.link_with(*this, (int)collision_object_type::door);
}

void medkit_world::link(enemy_object & e)
{
	e.link_with(*this, (int)collision_object_type::enemy);
}

#include "game.hpp"
#include <ctime>
#include "enemy.hpp"

game_world & game_world::ref()
{
	static game_world w;
	return w;
}

game_world::game_world() : rand(std::time(0))
{}

enemy_object & game_world::get_enemy(btCollisionObject const * obj)
{
	assert(obj && "invalid pointer");
	for (enemy_object * o : *_enemies)
		if (o->collision() == obj)
			return *o;
	throw std::logic_error{"unable to find a game-object"};
}

#include "game.hpp"

game_world & game_world::ref()
{
	static game_world w;
	return w;
}

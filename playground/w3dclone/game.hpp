//! herny svet
#pragma once
#include "physics/physics.hpp"

class game_object
{
public:
	virtual ~game_object() {}
	virtual btTransform const & transform() const = 0;
};

class game_world  // umoznuje interakiu hernych objektov
{
public:
	static game_world & ref();

	void update(float dt);
	game_object * player() const {return _player;}

	// zrejme iba docastne
	game_object * _player = nullptr;
};

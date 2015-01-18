#include "input.hpp"

static unsigned const NUM_KEYS = 256;
static bool keys[NUM_KEYS];
static bool keys_up[NUM_KEYS];

void input::init()
{
	for (int i = 0; i < NUM_KEYS; ++i)
		keys[i] = keys_up[i] = false;
}

void input::update()
{
	for (bool & key : keys_up)
		key = false;
}

bool input::key(int code)
{
	return keys[code];
}

bool input::key_up(int code)
{
	return keys_up[code];
}

void input::keyboard(unsigned char key, int x, int y)
{
	keys[key] = true;
}

void input::keyboard_up(unsigned char key, int x, int y)
{
	keys[key] = false;
	keys_up[key] = true;
}


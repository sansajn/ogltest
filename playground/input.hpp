#pragma once

class input
{
public:
	static void init();
	static void update();

	static bool key(int code);
	static bool key_up(int code);

	// glut
	static void keyboard(unsigned char key, int x, int y);
	static void keyboard_up(unsigned char key, int x, int y);
};

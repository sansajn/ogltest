#pragma once

class input
{
public:
	static void init();
	static void update();

	static bool key(int code);
	static bool key_up(int code);

	// glut
	static void glut_bind();
};

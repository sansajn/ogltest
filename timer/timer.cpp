#include "timer.h"

void timer::update()
{
	int const RATE = 100;

	if ((++_frames_elapsed % RATE) != 0)
		return;

	clock::time_point now = clock::now();
	clock::duration dt = now - _tp;
	float dt_ms = 
		std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();

	_fps = (RATE/dt_ms)*1000.0f;
	_tp = now;
	_frames_elapsed = 0;
}

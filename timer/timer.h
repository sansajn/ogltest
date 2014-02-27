#pragma once

#include <chrono>

namespace gl {

class timer
{
public:
	typedef std::chrono::high_resolution_clock clock;

	timer() : _tp(clock::now()), _fps(0.0f), _frames_elapsed(0) {}
	void update();
	float fps() const {return _fps;}

private:
	clock::time_point _tp;
	float _fps;
	unsigned int _frames_elapsed;
};

}  // gl

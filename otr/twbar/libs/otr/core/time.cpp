#include "time.hpp"
#include <chrono>

double now_in_ms()
{
	typedef std::chrono::high_resolution_clock clock;
	clock::time_point tp = clock::now();
	return double(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count());
}

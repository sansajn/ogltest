#include "sound.hpp"
#include <cassert>

namespace al {

device * default_device = nullptr;

void init_sound_system()
{
	init_sdl_audio();
	assert(default_device == 0 && "sound system already initialized");
	default_device = new al::device{44100, MIX_DEFAULT_FORMAT, 2, 4096};
}

void free_sound_system()
{
	if (default_device)
		delete default_device;
	quit_sdl();
}

}  // al

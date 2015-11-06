#pragma once
#include "al/audio.hpp"

namespace al {

void init_sound_system();
void free_sound_system();

extern device * default_device;

}  // al

#include "sound.hpp"
#include <cassert>
#include "al/audio.hpp"
#include "al/vorbis.hpp"
#include <AL/alc.h>

namespace al {

using std::shared_ptr;
using std::make_pair;

ALCdevice * __dev = nullptr;
ALCcontext * __ctx = nullptr;
device * __defdev = nullptr;
sound_manager * __sman = nullptr;


void init_sound_system()
{
	// device & context
	__dev = alcOpenDevice(nullptr);  // open default device
	assert(__dev && "failed to open a device");

	__ctx = alcCreateContext(__dev, nullptr);
	assert(__ctx && "failed to create an audio context");
	alcMakeContextCurrent(__ctx);
	assert(alGetError() == AL_NO_ERROR && "context stuff failed");
}

void free_sound_system()
{
	delete __defdev;
	delete __sman;

	if (__ctx)
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(__ctx);
	}

	if (__dev)
		alcCloseDevice(__dev);
}


device & device::ref()
{
	if (!__defdev)
		__defdev = new device{10};
	return *__defdev;
}

device::device(size_t source_count)
	: _sources(source_count)
{}

device::~device()
{}

void device::update()
{
	for (audio_source & s : _sources)
		s.update();
	_background_music_source.update();
}

audio_source * device::find_first_free_source()
{
	for (audio_source & s : _sources)
		if (!s.playing())
			return &s;

	return nullptr;
}

void device::play_effect(std::string const & sound_id, glm::vec3 const & position)
{
	audio_source * as = find_first_free_source();
	if (!as)
		return;  // audio not available, ignore request

	as->attach(sound_manager::ref().get(sound_id));
	as->play();
}

void device::play_music(std::string const & sound_id)
{
	_background_music_source.attach(sound_manager::ref().get(sound_id));
	_background_music_source.play();
}


sound_manager & sound_manager::ref()
{
	if (!__sman)
		__sman = new sound_manager{};
	return *__sman;
}

std::shared_ptr<wave_data> sound_manager::get(std::string const & sound_id)
{
	auto it = _sounds.find(sound_id);
	if (it != _sounds.end())
		return it->second;  // return value

	// not in cache, load from file as ogg
	shared_ptr<wave_data> data{new vorbis_wave{sound_id.c_str()}};
	auto result = _sounds.insert(make_pair(sound_id, data));
	return result.first->second;  // return value
}

}  // al

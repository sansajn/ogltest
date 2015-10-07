// prehravanie zvuku pomocou sdl
#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>

using namespace std;

char const * FILE_PATH = "testClip.wav";

namespace al {

class device
{
public:
	~device();
	void play(string const & fname);
	bool done() const;

private:
	static void audio_callback(void * userdata, uint8_t * stream, int length);

	SDL_AudioSpec _spec;
	uint8_t * _buf = nullptr;
	uint32_t _buflen = 0;
	uint32_t _bufidx = 0;
	SDL_AudioDeviceID _dev = 0;
};

device::~device()
{
	if (_dev)
		SDL_CloseAudioDevice(_dev);

	if (_buf)
		SDL_FreeWAV(_buf);
}

void device::play(string const & fname)
{
	if (!SDL_LoadWAV(fname.c_str(), &_spec, &_buf, &_buflen))
		throw logic_error{SDL_GetError()};

	_spec.callback = audio_callback;
	_spec.userdata = this;

	_dev = SDL_OpenAudioDevice(nullptr, 0, &_spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (!_dev)
		throw logic_error{SDL_GetError()};

	SDL_PauseAudioDevice(_dev, 0);
}

bool device::done() const
{
	return _bufidx >= _buflen;  // TODO: toto signalizuje iba ze som nakopiroval cely buffer do zariadenia
}

void device::audio_callback(void * userdata, uint8_t * stream, int length)
{
	device & self = *(device *)userdata;

	uint32_t len = min((uint32_t)length, self._buflen - self._bufidx);
	SDL_memcpy(stream, self._buf + self._bufidx, len);
	if (len < (uint32_t)length)
		memset(stream, 0, length - len);  // fill rest with silence

	self._bufidx += len;
}

}  // al


int main(int argc, char * argv[])
{
	SDL_Init(SDL_INIT_AUDIO);

	al::device dev;
	dev.play(FILE_PATH);

	SDL_Delay(1000);

	while (!dev.done())
		SDL_Delay(100);

	SDL_Quit();

	return 0;
}

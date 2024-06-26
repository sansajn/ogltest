// zvuk pomocou SDL_mixer (umoznuje prehrat niekolko zvukou naraz)
#include <map>
#include <stdexcept>
#include <string>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace std;

namespace al {

class device  //!< moze existovat iba jedna validna instancia v case
{
public:
	device(int frequency, uint16_t format, int channels, int chunksize);
	~device();
	void play_music(string const & fname);
	void play_effect(string const & fname);
	void joint() const;  //!< pocka pokial vsetko nedohraje, blokovacie volanie

	device(device const &) = delete;
	void operator=(device const &) = delete;

private:
	map<string, Mix_Music *> _musics;
	map<string, Mix_Chunk *> _effects;  // TODO: dealokuj nepouzivane buffre
	static bool initialized;
};

bool device::initialized = false;

device::device(int frequency, uint16_t format, int channels, int chunksize)
{
	if (initialized)
		throw logic_error{"already initialized"};

	Mix_Init(MIX_INIT_OGG|MIX_INIT_FLAC|MIX_INIT_MP3);
	initialized = true;

	if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0)
		throw logic_error(SDL_GetError());
}

device::~device()
{
	for (auto & kv : _musics)
		Mix_FreeMusic(kv.second);

	for (auto & kv : _effects)
		Mix_FreeChunk(kv.second);

	if (!initialized)
		return;

	Mix_CloseAudio();
	Mix_Quit();
	initialized = false;
}

void device::play_music(string const & fname)
{
	Mix_Music * music;
	auto it = _musics.find(fname);
	if (it != _musics.end())
		music = it->second;
	else
	{
		music = Mix_LoadMUS(fname.c_str());
		if (!music)
			throw logic_error{SDL_GetError()};
		_musics[fname] = music;
	}

	Mix_PlayMusic(music, 0);  // TODO: moznost ovplyvnit pocet opakovani repeat_mode: once, loop
}

void device::play_effect(string const & fname)
{
	Mix_Chunk * chunk;
	auto it = _effects.find(fname);
	if (it != _effects.end())
		chunk = it->second;
	else
	{
		chunk = Mix_LoadWAV(fname.c_str());
		if (!chunk)
			throw logic_error{SDL_GetError()};
		_effects[fname] = chunk;
	}

	Mix_PlayChannel(-1, chunk, 0);  // TODO: moznost ovplyvnit pocet opakovani repeat_mode: once, loop
}

void device::joint() const
{
	while (Mix_PlayingMusic() || Mix_Playing(-1))
		SDL_Delay(100);
}

}  // al


int main(int argc, char * argv[])
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw logic_error(SDL_GetError());

	al::device dev{44100, MIX_DEFAULT_FORMAT, 2, 4096};
	dev.play_music("testClip.wav");
	SDL_Delay(1000);
	dev.play_effect("fire01.ogg");
	dev.joint();

	SDL_Quit();
	return 0;
}

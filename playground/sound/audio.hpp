#include <map>
#include <string>
#include <SDL2/SDL_mixer.h>

namespace al {

class device  //!< moze existovat iba jedna validna instancia v case
{
public:
	device(int frequency, uint16_t format, int channels, int chunksize);
	~device();
	void play_music(std::string const & fname);
	void play_effect(std::string const & fname);
	void joint() const;  //!< pocka pokial vsetko nedohraje, blokovacie volanie

	device(device const &) = delete;
	void operator=(device const &) = delete;

private:
	std::map<std::string, Mix_Music *> _musics;
	std::map<std::string, Mix_Chunk *> _effects;  // TODO: dealokuj nepouzivane buffre
	static bool initialized;
};

void init_sdl_audio();  //!< sdl helpers
void quit_sdl();

}  // al

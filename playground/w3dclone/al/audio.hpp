#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include <SDL2/SDL_mixer.h>

namespace al {

/*!
\code
al::init_sdl_audio();
auto sound = new al::device{44100, MIX_DEFAULT_FORMAT, 2, 4096};
...
quit_sdl();
\endcode
\note moze existovat iba jedna validna instancia v case */
class device : private boost::noncopyable
{
public:
	device(int frequency, uint16_t format, int channels, int chunksize);
	~device();
	void play_music(std::string const & fname);  // TODO: chcem nastavit intenzitu/volume
	void play_effect(std::string const & fname);
	void joint() const;  //!< pocka pokial vsetko nedohraje, blokovacie volanie
	// TODO: potrebujem zistit, ci nieco hraje
	// TODO: ked dohraje music, chcem byt otom upozorneny
	// TODO: moznost prehrat len cast signalu
	// TODO: moznost posunut zaciatok prehravania

private:
	std::map<std::string, Mix_Music *> _musics;
	std::map<std::string, Mix_Chunk *> _effects;  // TODO: dealokuj nepouzivane buffre
	static bool initialized;
};

void init_sdl_audio();  //!< sdl helpers
void quit_sdl();

}  // al

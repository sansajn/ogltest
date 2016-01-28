#pragma once
#include <map>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include "al/audio.hpp"

namespace al {

void init_sound_system();
void free_sound_system();

class device  //! umoznuje sucasne prehratie N zvukou a 1 muziky na pozadi
{
public:
	static device & ref();
	~device();
	void update();
	void play_effect(std::string const & sound_id, glm::vec3 const & position = glm::vec3{0});
	void play_music(std::string const & sound_id);  // background music

private:
	device(size_t source_count);
	audio_source * find_first_free_source();

	std::vector<audio_source> _sources;
	audio_source _background_music_source;
};

class sound_manager  //!< map based sound manager with sound loading in ogg format
{
public:
	static sound_manager & ref();
	std::shared_ptr<wave_data> get(std::string const & sound_id);

private:
	std::map<std::string, std::shared_ptr<wave_data>> _sounds;
};

}  // al

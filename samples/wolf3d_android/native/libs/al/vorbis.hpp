#pragma once
#include <vector>
#include <string>
#include <cstdio>
#include <vorbis/vorbisfile.h>
#include "al/audio.hpp"

/*! Implementuje citanie vorbisu zo suboru.
Objekt typu vorbis_data je nezdielatelny napriec zdrojmi zvuku.
\note bez fungujucej implementacie read_at() nie mozne zdielat wave data medzi zdrojmi zvuku */
class vorbis_wave : public wave_data
{
public:
	vorbis_wave(char const * file_name);
	vorbis_wave(std::string const & file_name) : vorbis_wave{file_name.c_str()} {}
	~vorbis_wave();

	// wave_data api
	uint8_t * data() override;
	size_t stream_data(size_t size) override;
	size_t sample_rate() const override {return _vi->rate;}
	size_t channels() const override {return _vi->channels;}
	size_t bytes_per_sample() const override {return 2;}
	size_t sample_size() const override {return _sample_size;}
	void reset() override;

	size_t sample_count() const {return ov_pcm_total(const_cast<OggVorbis_File *>(&_vf), -1);}

private:
	FILE * _fin;
	OggVorbis_File _vf;
	vorbis_info * _vi;
	size_t _sample_size;
	std::vector<uint8_t> _data;
};

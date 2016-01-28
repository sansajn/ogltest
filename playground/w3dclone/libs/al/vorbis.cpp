#include "vorbis.hpp"
#include <stdexcept>
#include <cassert>

uint8_t * vorbis_wave::data()
{
	return _data.data();
}

size_t vorbis_wave::stream_data(size_t size)
{
	_data.resize(size);
	int current_section = 0;
	size_t total_read_bytes = 0;
	while (size > 0)
	{
		long read_bytes = ov_read(&_vf, (char *)(_data.data() + total_read_bytes), size, 0 /*little endian*/, bytes_per_sample(), 1, &current_section);
		assert(read_bytes >= 0 && "corrupt bitstream section");
		if (read_bytes == 0)
			break;  // eof
		size -= read_bytes;
		total_read_bytes += read_bytes;
	}
	return total_read_bytes;
}

void vorbis_wave::reset()
{
	int res = ov_pcm_seek(&_vf, 0);
	assert(res == 0 && "seek failed");
}

vorbis_wave::vorbis_wave(char const * file_name)
{
	_fin = fopen(file_name, "r");
	if (!_fin)
		throw std::runtime_error{"unable to open an input file"};

	int res = ov_open_callbacks(_fin, &_vf, NULL, 0, OV_CALLBACKS_NOCLOSE);
	assert(res == 0 && "ov_open_callbacks() failed");

	_vi = ov_info(&_vf, -1);
	assert(_vi && "ov_info() failed");

	_sample_size = bytes_per_sample() * _vi->channels;
}

vorbis_wave::~vorbis_wave()
{
	ov_clear(&_vf);
	fclose(_fin);
}

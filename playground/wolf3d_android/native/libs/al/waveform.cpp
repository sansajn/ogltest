#include "waveform.hpp"
#include <stdexcept>
#include <cassert>

using std::runtime_error;

waveform_wave::waveform_wave(char const * fname)
{
	_fin = fopen(fname, "r");
	if (!_fin)
		throw runtime_error{"can't open wav file"};

	fread(&_header, sizeof(detail::wav_header), 1, _fin);

	_bytes_per_sample = _header.bits_per_sample >> 3;
	_sample_size = _bytes_per_sample * _header.channels;
}

waveform_wave::~waveform_wave()
{
	fclose(_fin);
}

uint8_t * waveform_wave::data()
{
	return _data.data();
}

size_t waveform_wave::stream_data(size_t size)
{
	_data.resize(size);
	size_t read_bytes = fread(_data.data(), 1, size, _fin);
	if (read_bytes == 0)
		return 0;

	_offset += read_bytes;

	if (read_bytes < size)
		_data.resize(read_bytes);

	return read_bytes;
}

void waveform_wave::reset()
{
	fseek(_fin, sizeof(detail::wav_header), SEEK_SET);
}

size_t waveform_wave::channels() const
{
	return _header.channels;
}

size_t waveform_wave::sample_rate() const
{
	return _header.sample_rate;
}

size_t waveform_wave::bytes_per_sample() const
{
	return _bytes_per_sample;
}

size_t waveform_wave::sample_size() const
{
	return _sample_size;
}

#include "blob_wave.hpp"
#include <algorithm>
#include <cstring>

using std::min;

blob::blob(wave_data & wave)
{
	size_t const size = 4096 * wave.sample_size();

	while (1)
	{
		size_t off = _data.size();
		_data.resize(_data.size() + size);

		size_t read_bytes = wave.stream_data(size);
		if (read_bytes == 0)
			break;  // eof
		memcpy(_data.data() + off, wave.data(), read_bytes);

		if (read_bytes < size)
			_data.resize(off + read_bytes);
	}
}

uint8_t * blob::data(size_t offset)
{
	return _data.data() + offset;
}


blob_wave::blob_wave(std::shared_ptr<blob> samples, size_t sample_rate, size_t channels, size_t bytes_per_sample)
	: _samples{samples}
	, _rate{sample_rate}
	, _channels{channels}
	, _bytes_per_sample{bytes_per_sample}
	, _sample_size{_channels * bytes_per_sample}
	, _offset{0}
{}

uint8_t * blob_wave::data()
{
	return _samples->data(_offset);
}

size_t blob_wave::stream_data(size_t size)
{
	size_t read_bytes = min(_samples->size() - _offset, size);
	_offset += read_bytes;
	return read_bytes;
}

size_t blob_wave::sample_rate() const
{
	return _rate;
}

size_t blob_wave::channels() const
{
	return _channels;
}

size_t blob_wave::bytes_per_sample() const
{
	return _bytes_per_sample;
}

void blob_wave::reset()
{
	_offset = 0;
}

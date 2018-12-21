#pragma once
#include <vector>
#include <cstdio>
#include "al/audio.hpp"

namespace detail {

#pragma pack(push, 1)
struct __attribute__( ( packed, aligned( 1 ) ) ) wav_header
{
	uint8_t riff[4];
	uint32_t size;  // filesize-8 bytes
	uint8_t wave[4];
	uint8_t fmt[4];
	uint32_t size_fmt;
	uint16_t format_tag;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t avg_bytes_per_sec;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint8_t reserved[4];
	uint32_t data_size;
};
#pragma pack(pop)

}  // detail


class waveform_wave : public wave_data
{
public:
	waveform_wave(char const * fname);
	~waveform_wave();
	uint8_t * data() override;
	size_t stream_data(size_t size) override;
	size_t sample_rate() const override;
	size_t channels() const override;
	size_t bytes_per_sample() const override;
	size_t sample_size() const override;
	void reset() override;

private:
	FILE * _fin = nullptr;
	detail::wav_header _header;
	size_t _bytes_per_sample;
	size_t _sample_size;
	std::vector<uint8_t> _data;
	size_t _offset;
};

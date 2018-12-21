#pragma once
#include <memory>
#include <vector>
#include "al/audio.hpp"


class blob
{
public:
	blob(wave_data & wave);  // TODO: nech je toto externa funkcia
	uint8_t * data(size_t offset);
	size_t size() const {return _data.size();}

private:
	std::vector<uint8_t> _data;
};


class blob_wave : public wave_data
{
public:
	blob_wave(std::shared_ptr<blob> samples, size_t sample_rate, size_t channels, size_t bytes_per_sample);
	uint8_t * data() override;
	size_t stream_data(size_t size) override;
	size_t sample_rate() const override;
	size_t channels() const override;
	size_t bytes_per_sample() const override;
	void reset() override;

	size_t sample_count() const {return _samples->size();}

private:
	std::shared_ptr<blob> _samples;
	size_t _rate;
	size_t _channels;
	size_t _bytes_per_sample;
	size_t _sample_size;
	size_t _offset;
};

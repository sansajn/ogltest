#include "audio.hpp"
#include <thread>
#include <cassert>

audio_source::audio_source()
{
	alGenSources(1, &_source);
	_buffer_size = 0;
	_wave = nullptr;
}

void audio_source::play()
{
	assert(_wave && "not data attached");

	stop();
	unqueue_buffers_all();
	_wave->reset();

	size_t filled = fill_buffers(NUM_BUFFERS, _buffers);
	assert(filled > 0 && "nothing buffered");

	alSourceQueueBuffers(_source, filled, _buffers);
	alSourcePlay(_source);

	assert(alGetError() == AL_NO_ERROR && "openal: source queue failed");
}

void audio_source::update()
{
	if (!playing())
		return;

	assert(_wave && "wave data not initialized");

	ALuint unqueued[NUM_BUFFERS];
	int unqueue_count = unqueue_buffers(unqueued);
	if (unqueue_count == 0)
		return;

	size_t refilled = fill_buffers(unqueue_count, unqueued);

	alSourceQueueBuffers(_source, refilled, unqueued);
}

void audio_source::join()
{
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds{20});
		update();
	}
	while (playing());
}

void audio_source::attach(std::shared_ptr<wave_data> wave)
{
	free();
	_wave = wave;

	// buffers
	alGenBuffers(NUM_BUFFERS, _buffers);

	if (_wave->channels() > 1)
		_format = (_wave->bytes_per_sample() == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8);
	else
		_format = (_wave->bytes_per_sample() == 2 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8);

	_buffer_size = (BUFFER_TIME_MS * _wave->sample_rate() * _wave->sample_size()) / 1000;
}

audio_source::~audio_source()
{
	free();
	alDeleteSources(1, &_source);
}

void audio_source::stop()
{
	alSourceStop(_source);
}

bool audio_source::playing() const
{
	int state;
	alGetSourcei(_source, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

bool audio_source::paused() const
{
	int state;
	alGetSourcei(_source, AL_SOURCE_STATE, &state);
	return state == AL_PAUSED;
}

void audio_source::position(glm::vec3 const & p)
{
	alSource3f(_source, AL_POSITION, p.x, p.y, p.z);
}

void audio_source::velocity(glm::vec3 const & v)
{
	alSource3f(_source, AL_VELOCITY, v.x, v.y, v.z);
}

void audio_source::direction(glm::vec3 const & d)
{
	alSource3f(_source, AL_DIRECTION, d.x, d.y, d.z);
}

void audio_source::unqueue_buffers_all()
{
	stop();
	ALuint dummy[NUM_BUFFERS];
	unqueue_buffers(dummy);
}

int audio_source::unqueue_buffers(ALuint unqueued[NUM_BUFFERS])
{
	int processed;
	alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
	if (processed == 0)
		return 0;

	alSourceUnqueueBuffers(_source, processed, unqueued);
	assert(alGetError() == AL_NO_ERROR && "openal: unqueue buffers failed");

	return processed;
}

void audio_source::free()
{
	if (_wave)
	{
		unqueue_buffers_all();
		alDeleteBuffers(NUM_BUFFERS, _buffers);
		_buffer_size = 0;
	}

	assert(alGetError() == AL_NO_ERROR && "openal: release resource failed");
}

size_t audio_source::fill_buffers(size_t n, ALuint * buffers)
{
	for (size_t i = 0; i < n; ++i)
	{
		size_t read_bytes = _wave->stream_data(_buffer_size);
		if (read_bytes == 0)
			return i;  // eof
		alBufferData(buffers[i], _format, (ALvoid *)_wave->data(), read_bytes, _wave->sample_rate());

		assert(alGetError() == AL_NO_ERROR && "openal: filling buffer failed");
	}
	return n;
}

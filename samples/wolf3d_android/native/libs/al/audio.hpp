#pragma once
#include <memory>
#include <glm/vec3.hpp>
#include <AL/al.h>

/*! Rozhranie pre citanie zvuku.
\note zdielat data napriec zdrojmi zvuku je mozne iba v pripade implementacie read() citajucej data s urcenej pozicie */
class wave_data
{
public:
	virtual ~wave_data() {}
	virtual uint8_t * data() = 0;
	virtual size_t stream_data(size_t size) = 0;  //!< precita size bajtov zo vstupu
	virtual size_t sample_rate() const = 0;
	virtual size_t channels() const = 0;
	virtual size_t bytes_per_sample() const = 0;
	virtual size_t sample_size() const {return bytes_per_sample() * channels();}
	virtual void reset() = 0;  //!< umozni znova citat data
};

/*! Zdroj zvuku.
\note Zvuk nieje mozne pouzit vo viacerych zdrojoch sucastne. */
class audio_source
{
public:
	audio_source();
	~audio_source();
	void play();
	void attach(std::shared_ptr<wave_data> wave);
	void stop();
	bool playing() const;
	bool paused() const;
	void position(glm::vec3 const & p);
	void velocity(glm::vec3 const & v);
	void direction(glm::vec3 const & d);
	// TODO: properties(p,d)
	void update();
	void join();  //!< pocka kym sa neskonci prehravanie zvuku

private:
	static constexpr int NUM_BUFFERS = 4;
	static constexpr int BUFFER_TIME_MS = 200;

	void unqueue_buffers_all();
	int unqueue_buffers(ALuint unqueued[NUM_BUFFERS]);
	size_t fill_buffers(size_t n, ALuint * buffers);
	void free();

	ALuint _source;
	ALuint _buffers[NUM_BUFFERS];
	std::shared_ptr<wave_data> _wave;  //!< audio_source vlastni wave data
	size_t _buffer_size;  //!< in bytes
	ALenum _format;
};

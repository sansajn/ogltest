/*! \file utils.h
Štruktúry a funkcie, ktoré sa môžu hodiť. */
#pragma once
#include <cstdint>

/*! Umožnuje zápis heterogénnych dát do bloku pamäte. 
\ingroup utilities */
class memory_stream
{
public:
	memory_stream(size_t size) 
		: _buf(new uint8_t[size]), _cur(_buf), _size(size) 
	{}

	~memory_stream() {close();}

	template <typename T>
	memory_stream & operator<<(T const & val)
	{
		T * tbuf = (T *)_cur;
		*tbuf++ = val;
		_cur = (void *)tbuf;
		return *this;
	}

	template <typename R = void>
	R * buf(size_t offset = 0) const {return (R *)((uint8_t *)_buf + offset);}

	size_t size() const {return _size;}

	void close()
	{
		delete [] _buf;
		_buf = nullptr;
		_cur = nullptr;
		_size = 0;
	}

private:
	uint8_t * _buf;
	void * _cur;  // curent buffer position
	size_t _size;
};

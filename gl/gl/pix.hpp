#pragma once
#include <string>

namespace pix {  // pixel manipulation algorithms

void row_swap(uint8_t * a, uint8_t * b, unsigned len);
void flip(unsigned height, unsigned rowbytes, uint8_t * pixels);

struct decoder
{
	struct result_type
	{
		unsigned width;
		unsigned height;
		unsigned rowbytes;
		uint8_t * pixels;  //!< predpoklada alokaciu pomocou new[]
	};

	result_type result;

	decoder();
	virtual ~decoder();
	virtual void decode_as_rgba8(std::string const & fname) = 0;  // vola vynimku pri chybe
};  // decoder

}  // pix

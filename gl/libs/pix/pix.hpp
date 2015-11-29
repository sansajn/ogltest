#pragma once
#include <string>
#include <stdexcept>

namespace pix {  // pixel manipulation algorithms

using decoder_exception = std::runtime_error;

void row_swap(uint8_t * a, uint8_t * b, unsigned len);
void flip(unsigned height, unsigned rowbytes, uint8_t * pixels);

struct decoder
{
	struct result_type
	{
		unsigned width;
		unsigned height;
		unsigned rowbytes;
		uint8_t channels;
		uint8_t depth;  // 1 for 8-bit, 2 for 16-bit
		uint8_t * pixels;
	};

	result_type result;

	decoder();
	virtual ~decoder();
	virtual void decode(std::string const & fname) = 0;  //!< \note pri chybe vyvola vynimku
};  // decoder

}  // pix

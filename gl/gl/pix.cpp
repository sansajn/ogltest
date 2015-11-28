#include "pix.hpp"
#include <algorithm>
#include <cstring>
#include <cassert>

using std::swap;

namespace pix {  // pixel manipulation algorithms

void row_swap(uint8_t * a, uint8_t * b, unsigned len)
{
	for (unsigned i = 0; i < len; ++i)
		swap(*a++, *b++);
}

void flip(unsigned height, unsigned rowbytes, uint8_t * pixels)
{
	assert(height && rowbytes && pixels);
	unsigned half_rows = height/2;  // same as floor(h/2.0f)
	for (unsigned r = 0; r < half_rows; ++r)
	{
		uint8_t * a = pixels + r*rowbytes;
		uint8_t * b = pixels + (height-1-r)*rowbytes;
		row_swap(a, b, rowbytes);
	}
}

decoder::decoder()
{
	memset(&result, 0, sizeof(result_type));
}

decoder::~decoder()
{
	delete [] result.pixels;
}

}  // pix

#include "texture.h"
#include <algorithm>
#include <SOIL/SOIL.h>

namespace gl {

using std::swap;

static void invert_y(uint8_t * data, int w, int h, int ch);

texture::texture()
	: _w(0), _h(0), _ch(0), _data(nullptr)
{}

texture::texture(char const * filename)
	: texture()
{
	load(filename);
}

texture::~texture()
{
	if (_data)
		SOIL_free_image_data(_data);

	_data = nullptr;
}

bool texture::load(const char * filename)
{
	if (_data)
		SOIL_free_image_data(_data);

	_data = SOIL_load_image(filename, &_w, &_h, &_ch, SOIL_LOAD_RGBA);
	_ch = 4;
	if (_data)
		invert_y(_data, _w, _h, _ch);

	return _data;
}

void invert_y(uint8_t * data, int w, int h, int ch)
{
	for (int j = 0; j*2 < h; ++j)
	{
		int index1 = j * (w*ch);
		int index2 = (h - 1 - j) * (w*ch);
		for (int i = w; i > 0; --i)
		{
			for (int k = 0; k < ch; ++k)
				swap(data[index1++], data[index2++]);
		}
	}
}

}  // gl

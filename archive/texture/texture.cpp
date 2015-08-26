#include "texture.h"
#include <algorithm>
#include <cstring>
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

texture::texture(int w, int h, int ch)
	: _w(w), _h(h), _ch(ch)
{
	_data = new uint8_t[w*h*ch];
}

texture::texture(texture && rhs)
	: _data(rhs._data), _w(rhs._w), _h(rhs._h), _ch(rhs._ch)
{
	rhs._data = nullptr;
}

texture::~texture()
{
	free();
}

bool texture::load(const char * filename)
{
	free();

	uint8_t * image = SOIL_load_image(filename, &_w, &_h, &_ch, SOIL_LOAD_RGBA);
	if (!image)
		return false;
	_ch = 4;
	invert_y(image, _w, _h, _ch);

	_data = new uint8_t[_w*_h*_ch];
	memcpy((void *)_data, (void *)image, _w*_h*_ch);

	SOIL_free_image_data(image);

	return _data;
}

bool texture::load_heightmap(char const * filename)
{
	free();

	uint8_t * image = SOIL_load_image(filename, &_w, &_h, &_ch, SOIL_LOAD_L);
	if (!image)
		return false;
	_ch = 1;
	invert_y(image, _w, _h, _ch);

	_data = new uint8_t[_w*_h*_ch];
	memcpy((void *)_data, (void *)image, _w*_h*_ch);

	SOIL_free_image_data(image);

	return _data;
}

bool texture::save(char const * filename)
{
	return SOIL_save_image(filename, SOIL_SAVE_TYPE_BMP, _w, _h, _ch, _data) == 1;
}

void texture::free()
{
	if (_data)
		delete [] _data;
	_data = nullptr;
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

uint8_t * tile_get::operator()(int x, int y)
{
	int nx = x % _t.width();
	int ny = y % _t.height();
	return _t.data()+((ny*_t.width() + nx)*_t.channels());
}

uint8_t * scale_get::operator()(int x, int y)
{
	int nx = int((x/float(_w) * _t.width()) + 0.5f);
	int ny = int((y/float(_h) * _t.height()) + 0.5f);
	return _t.data()+((ny*_t.width())+nx);
}

}  // gl

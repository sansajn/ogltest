#pragma once

#include <cstdint>

namespace gl {

class texture
{
public:
	texture();
	texture(char const * filename);
	~texture();

	bool load(char const * filename);
	bool loaded() const {return _data;}
	int width() const {return _w;}
	int height() const {return _h;}
	uint8_t * data() const {return _data;}

private:
	int _w, _h, _ch;
	uint8_t * _data;
};

}  // gl

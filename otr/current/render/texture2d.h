#pragma once
#include "render/buffer.h"
#include "render/texture.h"

/*! Wrapper okolo #glTexImage2D.
\ingroup render */
class texture2D : public texture
{
public:
	texture2D(int w, int h, texture_internal_format fi, texture_format f,
		pixel_type t, parameters const & params, buffer::parameters const & s,
		buffer const & pixels);

	~texture2D() {}

	int width() const {return _w;}
	int height() const {return _h;}

	void image(int w, int h, texture_format f, pixel_type t,	buffer const & pixels);

	void subimage(int level, int x, int y, int w, int h, texture_format f,
		pixel_type t, buffer::parameters const & s, buffer const & pixels);

private:
	void generate_mipmap() {}

	int _w;
	int _h;
};

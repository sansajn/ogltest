#pragma once
#include "render/buffer.h"
#include "render/texture.hpp"

/*! \ingroup render */
class texture2D_array : public texture
{
public:
	texture2D_array(int w, int h, int l, texture_internal_format fi, texture_format f,
		pixel_type t, parameters const & params, buffer::parameters const & s, buffer const & pixels);

	~texture2D_array() {}

	int width() const {return _w;}
	int height() const {return _h;}
	int layers() const {return _l;}

	void subimage(int lod, int x, int y, int l, int w, int h, int d, texture_format f,
		pixel_type t, buffer::parameters const & s, buffer const & pixels);

	static int max_layers();

private:
	int _w;
	int _h;
	int _l;  //!< the number of layers
};

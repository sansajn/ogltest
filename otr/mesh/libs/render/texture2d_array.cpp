#include "render/texture2d_array.hpp"
#include <GL/glew.h>
#include "render/detail/texture_cast.hpp"

texture2D_array::texture2D_array(int w, int h, int l, texture_internal_format fi,
	texture_format f, pixel_type t, parameters const & params, buffer::parameters const & s,
	buffer const & pixels) : texture(GL_TEXTURE_2D_ARRAY, fi, params), _w(w), _h(h), _l(l)
{
	pixels.bind(GL_PIXEL_UNPACK_BUFFER);
	s.set();
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, ogl_cast(fi), w, h, l, 0, ogl_cast(f),
		ogl_cast(t), pixels.data(0));
	s.unset();
	pixels.unbind(GL_PIXEL_UNPACK_BUFFER);

	generate_mipmap();

	assert(glGetError() == GL_NO_ERROR);
}

void texture2D_array::subimage(int lod, int x, int y, int l, int w, int h, int d,
	texture_format f, pixel_type t, buffer::parameters const & s, buffer const & pixels)
{
	bind_to_texture_unit();
	pixels.bind(GL_PIXEL_UNPACK_BUFFER);
	s.set();
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, lod, x, y, l, w, h, d, ogl_cast(f),
		ogl_cast(t), pixels.data(0));
	s.unset();
	pixels.unbind(GL_PIXEL_UNPACK_BUFFER);

	assert(glGetError() == GL_NO_ERROR);
}

int texture2D_array::max_layers()
{
	GLint l;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &l);
	return l;
}

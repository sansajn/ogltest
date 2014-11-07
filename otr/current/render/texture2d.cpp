#include "render/texture2d.hpp"
#include <cassert>
#include <GL/glew.h>
#include "render/detail/texture_cast.hpp"

texture2D::texture2D(int w, int h, texture_internal_format fi,	texture_format f,
	pixel_type t, parameters const & params, buffer::parameters const & s,
	buffer const & pixels) : texture(GL_TEXTURE_2D, fi, params), _w(w), _h(h)
{
	pixels.bind(GL_PIXEL_UNPACK_BUFFER);
	s.set();
	glTexImage2D(target(), 0, ogl_cast(fi), w, h, 0, ogl_cast(f), ogl_cast(t), pixels.data(0));
	s.unset();
	pixels.unbind(GL_PIXEL_UNPACK_BUFFER);

	generate_mipmap();

	assert(glGetError() == GL_NO_ERROR);
}

void texture2D::image(int w, int h, texture_format f, pixel_type t, buffer const & pixels)
{
	_w = w;
	_h = h;

	bind_to_texture_unit();
	pixels.bind(GL_PIXEL_UNPACK_BUFFER);
	glTexImage2D(target(), 0, ogl_cast(internal_format()), w, h, 0, ogl_cast(f),
		ogl_cast(t), pixels.data(0));
	pixels.unbind(GL_PIXEL_UNPACK_BUFFER);

	generate_mipmap();

	assert(glGetError() == GL_NO_ERROR);
}


void texture2D::subimage(int level, int x, int y, int w, int h,
	texture_format f, pixel_type t, buffer::parameters const & s,
	buffer const & pixels)
{
	bind_to_texture_unit();
	pixels.bind(GL_PIXEL_UNPACK_BUFFER);
	s.set();
	glTexSubImage2D(target(), level, x, y, w, h, ogl_cast(f), ogl_cast(t),
		pixels.data(0));
	s.unset();
	pixels.unbind(GL_PIXEL_UNPACK_BUFFER);

	assert(glGetError() == GL_NO_ERROR);
}

#include "buffer.h"
#include <GL/glew.h>

buffer::parameters::parameters()
	: _swap_bytes(false), _least_significant_bit_first(false), _alignment(4), 
		_compressed_size(0), _sub_image2D(0, 0, 0), _sub_image3D(0, 0),_modified(false)
{}

buffer::parameters & buffer::parameters::swap_bytes(bool v)
{
	_swap_bytes = v;
	_modified = true;
	return *this;
}

buffer::parameters & buffer::parameters::least_significant_bit_first(bool v)
{

	_least_significant_bit_first = v;
	_modified = true;
	return *this;
}

buffer::parameters & buffer::parameters::alignment(GLint v)
{
	_alignment = v;
	_modified = true;
	return *this;
}

buffer::parameters & buffer::parameters::compressed_size(GLsizei size)
{
	_compressed_size = size;
	// do not set _modifid to true (???)
	return *this;
}

buffer::parameters & buffer::parameters::subimage2D(GLint skip_pixels, GLint skip_rows, GLint row_length)
{
	_sub_image2D = glm::ivec3(skip_pixels, skip_rows, row_length);
	_modified = true;
	return *this;
}

buffer::parameters & buffer::parameters::subimage3D(GLint skip_images, GLint image_height)
{
	_sub_image3D = glm::ivec2(skip_images, image_height);
	_modified = true;
	return *this;
}

void buffer::parameters::set() const
{
	if (!_modified)
		return;

	glPixelStorei(GL_UNPACK_SWAP_BYTES, _swap_bytes);
	glPixelStorei(GL_UNPACK_LSB_FIRST, _least_significant_bit_first);
	glPixelStorei(GL_UNPACK_ALIGNMENT, _alignment);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, _sub_image2D[0]);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, _sub_image2D[1]);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, _sub_image2D[2]);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, _sub_image3D[0]);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, _sub_image3D[1]);
}

void buffer::parameters::unset() const
{
	if (!_modified)
		return;

	glPixelStorei(GL_UNPACK_SWAP_BYTES, false);
	glPixelStorei(GL_UNPACK_LSB_FIRST, false);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
}


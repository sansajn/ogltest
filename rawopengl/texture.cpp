#include "texture.hpp"
#include <cassert>
#include <GL/glew.h>
#include <ImageMagick/Magick++.h>

texture::texture(std::string const & fname)
	: _tid(0)
{
	Magick::Image im(fname.c_str());
	Magick::Blob imblob;
	im.write(&imblob, "RGBA");

	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.columns(), im.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imblob.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

texture::texture(unsigned tid)
	: _tid(tid)
{
	assert(glIsTexture(tid) && "tid is not a texture id");

	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

texture::~texture()
{
	glDeleteTextures(1, &_tid);
}

void texture::bind(unsigned unit)
{
	assert(unit >= 0 && unit <= 31 && "not enougth texture units");  // TODO: zisti kolko mam texturovacich jednotiek
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, _tid);
}

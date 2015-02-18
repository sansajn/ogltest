#include "texture.hpp"
#include <cassert>
#include <GL/glew.h>
#include <ImageMagick/Magick++.h>

texture::texture() : _tid(0), _fid(0), _rid(0), _w(0), _h(0)
{}

texture::texture(std::string const & fname) : _tid(0), _fid(0), _rid(0)
{
	open(fname);
}

texture::texture(unsigned width, unsigned height) : _tid(0), _fid(0), _rid(0)
{
	create(width, height);
}

texture::texture(unsigned tid, unsigned width, unsigned height)
	: _tid(tid), _fid(0), _rid(0)
{
	assert(glIsTexture(tid) && "tid is not a texture id");

	_w = width;
	_h = height;

	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

texture::~texture()
{
	glDeleteTextures(1, &_tid);
	glDeleteRenderbuffers(1, &_rid);
	glDeleteFramebuffers(1, &_fid);
}

void texture::open(std::string const & fname)
{
	assert(!_tid && "texture already created");

	Magick::Image im(fname.c_str());
	im.flip();

	_w = im.columns();
	_h = im.rows();

	Magick::Blob imblob;  // TODO: treba ten obrazok kopirovat do blobu (nestaci blob s obrazku ?)
	im.write(&imblob, "RGBA");

	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imblob.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture::create(unsigned width, unsigned height)
{
	assert(!_tid && "texture already created");

	_w = width;
	_h = height;

	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _w, _h);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture::bind(unsigned unit)
{
	assert(unit >= 0 && unit <= 31 && "not enougth texture units");  // TODO: zisti kolko mam texturovacich jednotiek
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, _tid);
}

void texture::bind_as_render_target(bool depth)
{
	if (!_fid)
	{
		create_framebuffer();
		if (depth)
			create_depthbuffer();

		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::exception();
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fid);
	glViewport(0, 0, _w, _h);
}

void texture::create_framebuffer()
{
	glGenFramebuffers(1, &_fid);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fid);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tid, 0);
}

void texture::create_depthbuffer()
{
	glGenRenderbuffers(1, &_rid);
	glBindRenderbuffer(GL_RENDERBUFFER, _rid);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _w, _h);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rid);
}

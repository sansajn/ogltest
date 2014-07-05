#include <cassert>
#include "buffer.h"

gpubuffer::gpubuffer()
	: _size(0)
{
	glGenBuffers(1, &_buffid);
}

gpubuffer::~gpubuffer()
{
	glDeleteBuffers(1, &_buffid);
}

void gpubuffer::data(int size, void const * data, GLenum u)
{
	_size = size;
	glBindBuffer(GL_COPY_WRITE_BUFFER, _buffid);
	glBufferData(GL_COPY_WRITE_BUFFER, size, data, u);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void gpubuffer::subdata(int offset, int size, void * data)
{
	glBindBuffer(GL_COPY_WRITE_BUFFER, _buffid);
	glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

bool gpubuffer::reserve(int size, GLenum u)
{
	_size = size;
	glBindBuffer(GL_COPY_WRITE_BUFFER, _buffid);
	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, u);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");	
	return glGetError() == GL_NO_ERROR;
}

void gpubuffer::bind(GLenum target) const
{
	glBindBuffer(target, _buffid);
	assert(glGetError() == GL_NO_ERROR 
		&& "opengl error: unable to bind a buffer");
}

void gpubuffer::unbind(GLenum target) const
{
	glBindBuffer(target, 0);
	assert(glGetError() == GL_NO_ERROR 
		&& "opengl error: unable to unbind a buffer");
}

#include <cassert>
#include "render/meshbuffers.h"


attribute_buffer::attribute_buffer(int index, int size, GLenum t, 
	buffer_ptr b, int stride, int offset)
	: _index(index), _size(size), _type(t), _buf(b), _stride(stride), 
		_offset(offset)
{}

int attribute_buffer::attribute_size() const
{
	int type_size = 0;
	switch (_type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			type_size = 1;
			break;

		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:
			type_size = 2;
			break;

		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
			type_size = 4;
			break;

		case GL_DOUBLE:
			type_size = 8;
			break;

		case GL_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			type_size = 4;
			break;

		default:
			assert(false && "unknown attribute type");  // TODO: daj exception
	}
	return type_size * _size;
}


mesh_buffers const * mesh_buffers::CURRENT = nullptr;
GLenum mesh_buffers::_type = -1;


void mesh_buffers::append_attribute(int index, int size, int vertex_size,
	GLenum type, bool norm)
{
	int offset = 0;
	if (_attrs.size() > 0)
	{
		attrbuf_ptr a = _attrs.back();
		offset = a->offset() + a->attribute_size();
	}

	append_attribute(
		std::make_shared<attribute_buffer>(index, size, type,	nullptr, vertex_size, offset));
}

void mesh_buffers::draw(GLenum mode) const
{
	if (CURRENT != this)
		set();

	assert(_nprim != -1  && "error: forgot to set a number of primitives");

	if (_indices)
		glDrawElements(mode, _nprim, _type, 0);
	else
		glDrawArrays(mode, 0, _nprim);
}

void mesh_buffers::set() const
{
	if (CURRENT)
		CURRENT->unbind();
	bind();
	CURRENT = this;
}

void mesh_buffers::bind() const
{
	for (attrbuf_ptr const & a : _attrs)
	{
		attribute_buffer::buffer_ptr b = a->buf();
		b->bind(GL_ARRAY_BUFFER);
		glVertexAttribPointer(a->index(), a->size(), a->type(), GL_FALSE, a->stride(), b->data(a->offset()));
		glEnableVertexAttribArray(a->index());
	}
	assert(glGetError() == GL_NO_ERROR && "opengl error");

	if (_indices)
	{
		attribute_buffer::buffer_ptr b = _indices->buf();
		b->bind(GL_ELEMENT_ARRAY_BUFFER);
		_type = _indices->type();
	}
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh_buffers::unbind() const
{
	for (attrbuf_ptr const & a : _attrs)
		glDisableVertexAttribArray(a->index());
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh_buffers::reset()
{
	if (CURRENT == this)
	{
		unbind();
		CURRENT = nullptr;
	}
}


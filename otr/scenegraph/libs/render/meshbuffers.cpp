#include <cassert>
#include "render/meshbuffers.h"
#include "cast.h"

attribute_buffer::attribute_buffer(int index, int size, attribute_type t,
	ptr<buffer> b, int stride, int offset) : attribute_buffer(index, size, t, false, b, stride, offset)
{}

attribute_buffer::attribute_buffer(int index, int size, attribute_type t, bool norm,
	ptr<buffer> b, int stride, int offset) : _index(index), _size(size), _type(t),
		_buf(b), _stride(stride), _offset(offset), _norm(norm)
{}

int attribute_buffer::attribute_size() const
{
	int type_size = 0;
	switch (_type)
	{
		case attribute_type::A8I:
		case attribute_type::A8UI:
			type_size = 1;
			break;

		case attribute_type::A16I:
		case attribute_type::A16UI:
		case attribute_type::A16F:
			type_size = 2;
			break;

		case attribute_type::A32I:
		case attribute_type::A32UI:
		case attribute_type::A32F:
		case attribute_type::A32I_FIXED:
		case attribute_type::A32_2_10_10_10_REV:
		case attribute_type::A32UI_2_10_10_10_REV:
		case attribute_type::A32UI_10F_11F_11F_REV:
			type_size = 4;
			break;

		case attribute_type::A64F:
			type_size = 8;
			break;

		default:
			throw std::exception();  // TODO specify (unknown attribute type)
	}
	return type_size * _size;
}


mesh_buffers const * mesh_buffers::CURRENT = nullptr;
attribute_type mesh_buffers::_type = attribute_type::UNDEFINED;

mesh_buffers::mesh_buffers()
	: nvertices(0), nindices(0), mode(mesh_mode::TRIANGLES)
{}

mesh_buffers::~mesh_buffers()
{
	if (CURRENT == this)
	{
		unbind();
		CURRENT = nullptr;
	}
}

void mesh_buffers::append_attribute(int index, int size, int vertex_size,
	attribute_type type, bool norm)
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

void mesh_buffers::draw() const
{
	if (CURRENT != this)
		set();

	assert(primitive_count() > 0 && "error: forgot to set a nvertices or nindices");

	if (_indices)
		glDrawElements(ogl_cast(mode), primitive_count(), ogl_cast(_type), 0);  // TODO: implementuj offset (posledny argument)
	else
		glDrawArrays(ogl_cast(mode), 0, primitive_count());
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
		ptr<buffer> b = a->buf();
		b->bind(GL_ARRAY_BUFFER);
		glVertexAttribPointer(a->index(), a->size(), ogl_cast(a->type()), a->norm(),
			a->stride(), b->data(a->offset()));
		glEnableVertexAttribArray(a->index());
	}
	assert(glGetError() == GL_NO_ERROR && "opengl error");

	if (_indices)
	{
		ptr<buffer> b = _indices->buf();
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


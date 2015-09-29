#include "mesh_exp.hpp"
#include <algorithm>
#include <cassert>
#include <GL/glew.h>

using std::fill_n;
using std::swap;

namespace gl {

attribute::attribute(unsigned index, int size, int type, unsigned stride, int start_idx, int normalized)
	: index{index}, size{size}, type{type}, normalized{normalized}, stride{stride}, start_idx{start_idx}
{
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			int_type = true;
			break;

		default:
			int_type = false;
	}
}

mesh::mesh()
	: _nindices{0}, _draw_mode{GL_TRIANGLES}
{
	fill_n(_gpu_buffer_ids, buffer_count, 0);
}

mesh::mesh(void const * vbuf, unsigned vbuf_sizeof, unsigned const * ibuf, unsigned ibuf_size)
	: _nindices{ibuf_size}, _draw_mode{GL_TRIANGLES}
{
	fill_n(_gpu_buffer_ids, buffer_count, 0);

	glGenBuffers(buffer_count, _gpu_buffer_ids);
	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_ids[vbo_id]);
	glBufferData(GL_ARRAY_BUFFER, vbuf_sizeof, vbuf, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_ids[ibo_id]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibuf_size*sizeof(unsigned), ibuf, GL_STATIC_DRAW);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh::mesh(mesh && other)
{
	_nindices = other._nindices;
	swap(_attribs, other._attribs);
	_gpu_buffer_ids[vbo_id] = other._gpu_buffer_ids[vbo_id];
	_gpu_buffer_ids[ibo_id] = other._gpu_buffer_ids[ibo_id];
	other._gpu_buffer_ids[vbo_id] = other._gpu_buffer_ids[ibo_id] = 0;
	_draw_mode = other._draw_mode;
}

mesh::~mesh()
{
	glDeleteBuffers(buffer_count, _gpu_buffer_ids);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::append_attribute(attribute const & a)
{
	_attribs.push_back(a);
}

void mesh::draw_mode(int mode)
{
	_draw_mode = mode;
}

void mesh::render() const
{
	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_ids[vbo_id]);  // vertices

	for (attribute const & a : _attribs)
	{
		glEnableVertexAttribArray(a.index);
		if (a.int_type)
			glVertexAttribIPointer(a.index, a.size, a.type, a.stride, (GLvoid *)(a.start_idx));
		else
			glVertexAttribPointer(a.index, a.size, a.type, a.normalized, a.stride, (GLvoid *)(a.start_idx));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_ids[ibo_id]);  // indices

	glDrawElements(_draw_mode, _nindices, GL_UNSIGNED_INT, 0);

	for (attribute const & a : _attribs)
		glDisableVertexAttribArray(a.index);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::operator=(mesh && other)
{
	swap(_nindices, other._nindices);
	swap(_attribs, other._attribs);
	swap(_gpu_buffer_ids[vbo_id], other._gpu_buffer_ids[vbo_id]);
	swap(_gpu_buffer_ids[ibo_id], other._gpu_buffer_ids[ibo_id]);
	swap(_draw_mode, other._draw_mode);
}

}  // gl

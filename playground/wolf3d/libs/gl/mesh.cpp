#include "mesh.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <GL/glew.h>

using std::swap;
using std::move;
using std::vector;
using std::runtime_error;
using std::logic_error;

namespace gl {

GLenum opengl_cast(buffer_usage u);
GLenum opengl_cast(buffer_target t);
GLenum opengl_cast(render_primitive p);

gpu_buffer::gpu_buffer(size_t size, buffer_usage usage)
{
	glGenBuffers(1, &_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, opengl_cast(usage));
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);  // unbind
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

gpu_buffer::gpu_buffer(void const * buf, size_t size, buffer_usage usage)
{
	glGenBuffers(1, &_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferData(GL_COPY_WRITE_BUFFER, size, buf, opengl_cast(usage));
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);  // unbind
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

gpu_buffer::gpu_buffer(gpu_buffer && other)
	: _id{other._id}
{
	other._id = 0;
}

void gpu_buffer::operator=(gpu_buffer && other)
{
	swap(_id, other._id);
}

gpu_buffer::~gpu_buffer()
{
	if (_id)
		glDeleteBuffers(1, &_id);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void gpu_buffer::data(void const * buf, size_t size, unsigned offset)
{
	assert(_id && "uninitialized buffer");
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, buf);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

unsigned gpu_buffer::id() const
{
	return _id;
}

void gpu_buffer::bind(buffer_target target) const
{
	assert(_id && "uninitialized buffer");
	glBindBuffer(opengl_cast(target), _id);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

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

mesh::mesh() : _vao{0}
{}

mesh::mesh(size_t vbuf_size_in_bytes, size_t index_count, buffer_usage usage)
	: _vao{0}, _vbuf{vbuf_size_in_bytes, usage}, _ibuf(index_count*sizeof(unsigned), usage), _nindices{index_count}, _draw_mode{GL_TRIANGLES}
{}

mesh::mesh(void const * vbuf, size_t vbuf_size, unsigned const * ibuf, size_t ibuf_size, buffer_usage usage)
	: _vao{0}, _vbuf{vbuf, vbuf_size, usage}, _ibuf(ibuf, ibuf_size*sizeof(unsigned), usage), _nindices{ibuf_size}, _draw_mode{GL_TRIANGLES}
{}

mesh::mesh(mesh && other)
	: _vao{other._vao}
	, _vbuf{move(other._vbuf)}
	, _ibuf{move(other._ibuf)}
	, _nindices{other._nindices}
	, _draw_mode{other._draw_mode}
{
	other._vao = 0;
}

void mesh::operator=(mesh && other)
{
	swap(_vao, other._vao);
	_nindices = other._nindices;
	_vbuf = move(other._vbuf);
	_ibuf = move(other._ibuf);
	_draw_mode = other._draw_mode;
}

mesh::~mesh()
{
	glDeleteVertexArrays(1, &_vao);
}

void mesh::render() const
{
	assert(_vao && "attributes not attached");
	glBindVertexArray(_vao);
	glDrawElements(_draw_mode, _nindices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);  // unbind vao
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::attach_attributes(std::initializer_list<vertex_attribute_type> attribs)
{
	// setup vertex array object for later use
	assert(!_vao && "attributes already attached");

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_vbuf.bind(gl::buffer_target::array);
	_ibuf.bind(gl::buffer_target::element_array);

	for (vertex_attribute_type const & a : attribs)
	{
		if (a.int_type)
			glVertexAttribIPointer(a.index, a.size, a.type, a.stride, (GLvoid *)(intptr_t)a.start_idx);
		else
			glVertexAttribPointer(a.index, a.size, a.type, a.normalized, a.stride, (GLvoid *)(intptr_t)a.start_idx);

		glEnableVertexAttribArray(a.index);
	}

	glBindVertexArray(0);  // unbind vao
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::draw_mode(render_primitive_type mode)
{
	_draw_mode = opengl_cast(mode);
}

void mesh::data(void const * vsubbuf, unsigned size, unsigned offset)
{
	_vbuf.data(vsubbuf, size, offset);
}

void mesh::data(void const * vbuf, unsigned vbuf_size, unsigned const * ibuf, unsigned ibuf_size)
{
	data(vbuf, vbuf_size, 0, ibuf, ibuf_size, 0);
}

void mesh::data(void const * vsubbuf, unsigned vsubbuf_size, unsigned vsubbuf_offset, unsigned const * isubbuf, unsigned isubbuf_size, unsigned isubbuf_offset)
{
	_vbuf.data(vsubbuf, vsubbuf_size, vsubbuf_offset);
	_ibuf.data(isubbuf, isubbuf_size*sizeof(unsigned), isubbuf_offset);
}


mesh mesh_from_vertices(std::vector<vertex> const & verts, std::vector<unsigned> const & indices)
{
	vector<float> vbuf;
	vbuf.resize(verts.size()*(3+2+3+3));

	float * fptr = vbuf.data();
	for (vertex const & v : verts)
	{
		*fptr++ = v.position.x;
		*fptr++ = v.position.y;
		*fptr++ = v.position.z;
		*fptr++ = v.uv.x;
		*fptr++ = v.uv.y;
		*fptr++ = v.normal.x;
		*fptr++ = v.normal.y;
		*fptr++ = v.normal.z;
		*fptr++ = v.tangent.x;
		*fptr++ = v.tangent.y;
		*fptr++ = v.tangent.z;
	}

	mesh m(vbuf.data(), vbuf.size()*sizeof(float), indices.data(), indices.size());
	// TODO: vertex by mal poskytnut attributy
	unsigned stride = (3+2+3+3)*sizeof(GLfloat);
	m.attach_attributes({
		attribute{0, 3, GL_FLOAT, stride},  // position
		attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GLfloat)},  // texcoord
		attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GLfloat)},  // normal
		attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GLfloat)}  // tangent
	});

	return m;
}


GLenum opengl_cast(buffer_usage u)
{
	switch (u)
	{
		case buffer_usage::stream_draw: return GL_STREAM_DRAW;
		case buffer_usage::stream_read: return GL_STREAM_READ;
		case buffer_usage::stream_copy: return GL_STREAM_COPY;
		case buffer_usage::static_draw: return GL_STATIC_DRAW;
		case buffer_usage::static_read: return GL_STATIC_READ;
		case buffer_usage::static_copy: return GL_STATIC_COPY;
		case buffer_usage::dynamic_draw: return GL_DYNAMIC_DRAW;
		case buffer_usage::dynamic_read: return GL_DYNAMIC_READ;
		case buffer_usage::dynamic_copy: return GL_DYNAMIC_COPY;
		default:
			throw logic_error{"bad cast, unknown buffer usage"};
	}
}

GLenum opengl_cast(buffer_target t)
{
	switch (t)
	{
		case buffer_target::array: return GL_ARRAY_BUFFER;
		case buffer_target::atomic_counter: return GL_ATOMIC_COUNTER_BUFFER;
		case buffer_target::copy_read: return GL_COPY_READ_BUFFER;
		case buffer_target::copy_write: return GL_COPY_WRITE_BUFFER;
		case buffer_target::draw_indirect: return GL_DRAW_INDIRECT_BUFFER;
		case buffer_target::dispatch_indirect: return GL_DISPATCH_INDIRECT_BUFFER;
		case buffer_target::element_array: return GL_ELEMENT_ARRAY_BUFFER;
		case buffer_target::pixel_pack: return GL_PIXEL_PACK_BUFFER;
		case buffer_target::pixel_unpack: return GL_PIXEL_UNPACK_BUFFER;
		case buffer_target::query: return GL_QUERY_BUFFER;
		case buffer_target::shader_storage: return GL_SHADER_STORAGE_BUFFER;
		case buffer_target::texture: return GL_TEXTURE_BUFFER;
		case buffer_target::transform_feedback: return GL_TRANSFORM_FEEDBACK_BUFFER;
		case buffer_target::uniform: return GL_UNIFORM_BUFFER;
		default:
			throw logic_error{"bad cast, unknown buffer target"};
	}
}

GLenum opengl_cast(render_primitive p)
{
	switch (p)
	{
		case render_primitive::points: return GL_POINTS;
		case render_primitive::line_strip: return GL_LINE_STRIP;
		case render_primitive::line_loop: return GL_LINE_LOOP;
		case render_primitive::lines: return GL_LINES;
		case render_primitive::line_strip_adjacency: return GL_LINE_STRIP_ADJACENCY;
		case render_primitive::lines_adjacency: return GL_LINES_ADJACENCY;
		case render_primitive::triangle_strip: return GL_TRIANGLE_STRIP;
		case render_primitive::triangle_fan: return GL_TRIANGLE_FAN;
		case render_primitive::triangles: return GL_TRIANGLES;
		case render_primitive::triangle_strip_adjacency: return GL_TRIANGLE_STRIP_ADJACENCY;
		case render_primitive::triangles_adjacency: return GL_TRIANGLES_ADJACENCY;
		case render_primitive::patches: return GL_PATCHES;
		default:
			throw logic_error{"bad cast, unknown render primitive"};
	}
}

}  // gl

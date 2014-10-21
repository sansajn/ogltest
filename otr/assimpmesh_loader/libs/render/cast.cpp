#include "cast.h"
#include <exception>

GLenum ogl_cast(mesh_mode m)
{
	switch (m)
	{
		case mesh_mode::triangles:
			return GL_TRIANGLES;
		case mesh_mode::triangle_strip:
			return GL_TRIANGLE_STRIP;
		case mesh_mode::triangle_fun:
			return GL_TRIANGLE_FAN;
		case mesh_mode::points:
			return GL_POINTS;
		case mesh_mode::line_strip:
			return GL_LINE_STRIP;
		case mesh_mode::line_loop:
			return GL_LINE_LOOP;
		case mesh_mode::lines:
			return GL_LINES;
		case mesh_mode::line_strip_adjacency:
			return GL_LINE_STRIP_ADJACENCY;
		case mesh_mode::lines_adjacency:
			return GL_LINES_ADJACENCY;
		case mesh_mode::triangle_strip_adjacency:
			return GL_TRIANGLE_STRIP_ADJACENCY;
		case mesh_mode::triangles_adjacency:
			return GL_TRIANGLES_ADJACENCY;
		case mesh_mode::patches:
			return GL_PATCHES;
		default:
			throw std::exception();  // TODO specify (unsupported primitive type)
	}
}

GLenum ogl_cast(polygon_rasterization r)
{
	switch (r)
	{
		case polygon_rasterization::POINT:
			return GL_POINT;
		case polygon_rasterization::LINE:
			return GL_LINE;
		case polygon_rasterization::FILL:
			return GL_FILL;
		default:
			throw std::exception();  // TODO specify
	}
}

GLenum ogl_cast(attribute_type t)
{
	switch (t)
	{
		case attribute_type::i8:
			return GL_BYTE;
		case attribute_type::ui8:
			return GL_UNSIGNED_BYTE;
		case attribute_type::i16:
			return GL_SHORT;
		case attribute_type::ui16:
			return GL_UNSIGNED_SHORT;
		case attribute_type::i32:
			return GL_INT;
		case attribute_type::ui32:
			return GL_UNSIGNED_INT;
		case attribute_type::f16:
			return GL_HALF_FLOAT;
		case attribute_type::f32:
			return GL_FLOAT;
		case attribute_type::f64:
			return GL_DOUBLE;
		case attribute_type::i32_fixed:
			return GL_FIXED;
		case attribute_type::i32_2_10_10_10_rev:
			return GL_INT_2_10_10_10_REV;
		case attribute_type::ui32_2_10_10_10_rev:
			return GL_UNSIGNED_INT_2_10_10_10_REV;
		case attribute_type::ui32_10F_11F_11F_rev:
			return GL_UNSIGNED_INT_10F_11F_11F_REV;
		default:
			throw std::exception();  // TODO: specify (unknown attribute type)
	}
}

GLenum ogl_cast(buffer_usage u)
{
	switch (u)
	{
		case buffer_usage::STREAM_DRAW:
			return GL_STREAM_DRAW;
		case buffer_usage::STREAM_READ:
			return GL_STREAM_READ;
		case buffer_usage::STREAM_COPY:
			return GL_STREAM_COPY;
		case buffer_usage::STATIC_DRAW:
			return GL_STATIC_DRAW;
		case buffer_usage::STATIC_READ:
			return GL_STATIC_READ;
		case buffer_usage::STATIC_COPY:
			return GL_STATIC_COPY;
		case buffer_usage::DYNAMIC_DRAW:
			return GL_DYNAMIC_DRAW;
		case buffer_usage::DYNAMIC_READ:
			return GL_DYNAMIC_READ;
		case buffer_usage::DYNAMIC_COPY:
			return GL_DYNAMIC_COPY;
		default:
			throw std::exception();  // TODO specify (unknown buffer usage)
	}  // switch
}

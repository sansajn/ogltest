#include "cast.h"
#include <exception>

GLenum ogl_cast(mesh_mode m)
{
	switch (m)
	{
		case mesh_mode::TRIANGLES:
			return GL_TRIANGLES;
		case mesh_mode::TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		case mesh_mode::TRIANGLE_FAN:
			return GL_TRIANGLE_FAN;
		case mesh_mode::POINTS:
			return GL_POINTS;
		case mesh_mode::LINE_STRIP:
			return GL_LINE_STRIP;
		case mesh_mode::LINE_LOOP:
			return GL_LINE_LOOP;
		case mesh_mode::LINES:
			return GL_LINES;
		case mesh_mode::LINE_STRIP_ADJACENCY:
			return GL_LINE_STRIP_ADJACENCY;
		case mesh_mode::LINES_ADJACENCY:
			return GL_LINES_ADJACENCY;
		case mesh_mode::TRIANGLE_STRIP_ADJACENCY:
			return GL_TRIANGLE_STRIP_ADJACENCY;
		case mesh_mode::TRIANGLES_ADJACENCY:
			return GL_TRIANGLES_ADJACENCY;
		case mesh_mode::PATCHES:
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
		case attribute_type::A8I:
			return GL_BYTE;
		case attribute_type::A8UI:
			return GL_UNSIGNED_BYTE;
		case attribute_type::A16I:
			return GL_SHORT;
		case attribute_type::A16UI:
			return GL_UNSIGNED_SHORT;
		case attribute_type::A32I:
			return GL_INT;
		case attribute_type::A32UI:
			return GL_UNSIGNED_INT;
		case attribute_type::A16F:
			return GL_HALF_FLOAT;
		case attribute_type::A32F:
			return GL_FLOAT;
		case attribute_type::A64F:
			return GL_DOUBLE;
		case attribute_type::A32I_FIXED:
			return GL_FIXED;
		case attribute_type::A32_2_10_10_10_REV:
			return GL_INT_2_10_10_10_REV;
		case attribute_type::A32UI_2_10_10_10_REV:
			return GL_UNSIGNED_INT_2_10_10_10_REV;
		case attribute_type::A32UI_10F_11F_11F_REV:
			return GL_UNSIGNED_INT_10F_11F_11F_REV;
		default:
			throw std::exception();  // TODO specify (unknown attribute type)
	}
}

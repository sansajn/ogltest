#include "render/cast.hpp"
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

GLenum ogl_cast(polygon_raster_mode r)
{
	switch (r)
	{
		case polygon_raster_mode::POINT:
			return GL_POINT;
		case polygon_raster_mode::LINE:
			return GL_LINE;
		case polygon_raster_mode::FILL:
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

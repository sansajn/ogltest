#include "render/detail/texture_cast.hpp"
#include <exception>

GLenum ogl_cast(texture_internal_format fi)
{
	switch (fi)
	{
		case texture_internal_format::DEPTH_COMPONENT:
			return GL_DEPTH_COMPONENT;
		case texture_internal_format::DEPTH_STENCIL:
			return GL_DEPTH_STENCIL;
		case texture_internal_format::RED:
			return GL_RED;
		case texture_internal_format::RG:
			return GL_RG;
		case texture_internal_format::RGB:
			return GL_RGB;
		case texture_internal_format::RGBA:
			return GL_RGBA;

		// sized
		case texture_internal_format::R8:
			return GL_R8;
		case texture_internal_format::R8_SNORM:
			return GL_R8_SNORM;
		case texture_internal_format::R16:
			return GL_R16;
		case texture_internal_format::R16_SNORM:
			return GL_R16_SNORM;
		case texture_internal_format::RG8:
			return GL_RG8;
		case texture_internal_format::RG8_SNORM:
			return GL_RG8_SNORM;
		case texture_internal_format::RG16:
			return GL_RG16;
		case texture_internal_format::RG16_SNORM:
			return GL_RG16_SNORM;
		case texture_internal_format::R3_G3_B2:
			return GL_R3_G3_B2;
		case texture_internal_format::RGB4:
			return GL_RGB4;
		case texture_internal_format::RGB5:
			return GL_RGB5;
		case texture_internal_format::RGB8:
			return GL_RGB8;
		case texture_internal_format::RGB8_SNORM:
			return GL_RGB8_SNORM;
		case texture_internal_format::RGB10:
			return GL_RGB10;
		case texture_internal_format::RGB12:
			return GL_RGB12;
		case texture_internal_format::RGB16_SNORM:
			return GL_RGB16_SNORM;
		case texture_internal_format::RGBA2:
			return GL_RGBA2;
		case texture_internal_format::RGBA4:
			return GL_RGBA4;
		case texture_internal_format::RGB5_A1:
			return GL_RGB5_A1;
		case texture_internal_format::RGBA8:
			return GL_RGBA8;
		case texture_internal_format::RGBA8_SNORM:
			return GL_RGBA8_SNORM;
		case texture_internal_format::RGB10_A2:
			return GL_RGB10_A2;
		case texture_internal_format::RGB10_A2UI:
			return GL_RGB10_A2UI;
		case texture_internal_format::RGBA12:
			return GL_RGBA12;
		case texture_internal_format::RGBA16:
			return GL_RGBA16;
		case texture_internal_format::SRGB8:
			return GL_SRGB8;
		case texture_internal_format::SRGB8_ALPHA8:
			return GL_SRGB8_ALPHA8;
		case texture_internal_format::R16F:
			return GL_R16F;
		case texture_internal_format::RG16F:
			return GL_RG16F;
		case texture_internal_format::RGB16F:
			return GL_RGB16F;
		case texture_internal_format::RGBA16F:
			return GL_RGBA16F;
		case texture_internal_format::R32F:
			return GL_R32F;
		case texture_internal_format::RG32F:
			return GL_RG32F;
		case texture_internal_format::RGB32F:
			return GL_RGB32F;
		case texture_internal_format::RGBA32F:
			return GL_RGBA32F;
		case texture_internal_format::R11F_G11F_B10F:
			return GL_R11F_G11F_B10F;
		case texture_internal_format::RGB9_E5:
			return GL_RGB9_E5;
		case texture_internal_format::R8I:
			return GL_R8I;
		case texture_internal_format::R8UI:
			return GL_R8UI;
		case texture_internal_format::R16I:
			return GL_R16I;
		case texture_internal_format::R16UI:
			return GL_R16UI;
		case texture_internal_format::R32I:
			return GL_R32I;
		case texture_internal_format::R32UI:
			return GL_R32UI;
		case texture_internal_format::RG8I:
			return GL_RG8I;
		case texture_internal_format::RG8UI:
			return GL_RG8UI;
		case texture_internal_format::RG16I:
			return GL_RG16I;
		case texture_internal_format::RG16UI:
			return GL_RG16UI;
		case texture_internal_format::RG32I:
			return GL_RG32I;
		case texture_internal_format::RG32UI:
			return GL_RG32UI;
		case texture_internal_format::RGB8I:
			return GL_RGB8I;
		case texture_internal_format::RGB8UI:
			return GL_RGB8UI;
		case texture_internal_format::RGB16I:
			return GL_RGB16I;
		case texture_internal_format::RGB16UI:
			return GL_RGB16UI;
		case texture_internal_format::RGB32I:
			return GL_RGB32I;
		case texture_internal_format::RGB32UI:
			return GL_RGB32UI;
		case texture_internal_format::RGBA8I:
			return GL_RGBA8I;
		case texture_internal_format::RGBA8UI:
			return GL_RGBA8UI;
		case texture_internal_format::RGBA16I:
			return GL_RGBA16I;
		case texture_internal_format::RGBA16UI:
			return GL_RGBA16UI;
		case texture_internal_format::RGBA32I:
			return GL_RGBA32I;
		case texture_internal_format::RGBA32UI:
			return GL_RGBA32UI;

		// compressed
		case texture_internal_format::COMPRESSED_RED:
			return GL_COMPRESSED_RED;
		case texture_internal_format::COMPRESSED_RG:
			return GL_COMPRESSED_RG;
		case texture_internal_format::COMPRESSED_RGB:
			return GL_COMPRESSED_RGB;
		case texture_internal_format::COMPRESSED_RGBA:
			return GL_COMPRESSED_RGBA;
		case texture_internal_format::COMPRESSED_SRGB:
			return GL_COMPRESSED_SRGB;
		case texture_internal_format::COMPRESSED_SRGB_ALPHA:
			return GL_COMPRESSED_SRGB_ALPHA;
		case texture_internal_format::COMPRESSED_RED_RGTC1:
			return GL_COMPRESSED_RED_RGTC1;
		case texture_internal_format::COMPRESSED_SIGNED_RED_RGTC1:
			return GL_COMPRESSED_SIGNED_RED_RGTC1;
		case texture_internal_format::COMPRESSED_RG_RGTC2:
			return GL_COMPRESSED_RG_RGTC2;
		case texture_internal_format::COMPRESSED_SIGNED_RG_RGTC2:
			return GL_COMPRESSED_SIGNED_RG_RGTC2;
		case texture_internal_format::COMPRESSED_RGBA_BPTC_UNORM:
			return GL_COMPRESSED_RGBA_BPTC_UNORM;
		case texture_internal_format::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
		case texture_internal_format::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
			return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
		case texture_internal_format::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;

		default:
			throw std::exception();  // TODO specify
	};  // switch
}

GLenum ogl_cast(texture_format f)
{
	switch (f)
	{
		case texture_format::RED:
			return GL_RED;
		case texture_format::RG:
			return GL_RG;
		case texture_format::RGB:
			return GL_RGB;
		case texture_format::BGR:
			return GL_BGR;
		case texture_format::RGBA:
			return GL_RGBA;
		case texture_format::BGRA:
			return GL_BGRA;
		case texture_format::RED_INTEGER:
			return GL_RED_INTEGER;
		case texture_format::RG_INTEGER:
			return GL_RG_INTEGER;
		case texture_format::RGB_INTEGER:
			return GL_RGB_INTEGER;
		case texture_format::BGR_INTEGER:
			return GL_BGR_INTEGER;
		case texture_format::RGBA_INTEGER:
			return GL_RGBA_INTEGER;
		case texture_format::BGRA_INTEGER:
			return GL_BGRA_INTEGER;
		case texture_format::STENCIL_INDEX:
			return GL_STENCIL_INDEX;
		case texture_format::DEPTH_COMPONENT:
			return GL_DEPTH_COMPONENT;
		case texture_format::DEPTH_STENCIL:
			return GL_DEPTH_STENCIL;
		default:
			throw std::exception();  // TODO: specify
	}
}

GLenum ogl_cast(pixel_type t)
{
	switch (t)
	{
		case pixel_type::UNSIGNED_BYTE:
			return GL_UNSIGNED_BYTE;
		case pixel_type::BYTE:
			return GL_BYTE;
		case pixel_type::UNSIGNED_SHORT:
			return GL_UNSIGNED_SHORT;
		case pixel_type::SHORT:
			return GL_SHORT;
		case pixel_type::UNSIGNED_INT:
			return GL_UNSIGNED_INT;
		case pixel_type::INT:
			return GL_INT;
		case pixel_type::FLOAT:
			return GL_FLOAT;
		case pixel_type::UNSIGNED_BYTE_3_3_2:
			return GL_UNSIGNED_BYTE_3_3_2;
		case pixel_type::UNSIGNED_BYTE_2_3_3_REV:
			return GL_UNSIGNED_BYTE_2_3_3_REV;
		case pixel_type::UNSIGNED_SHORT_5_6_5:
			return GL_UNSIGNED_SHORT_5_6_5;
		case pixel_type::UNSIGNED_SHORT_5_6_5_REV:
			return GL_UNSIGNED_SHORT_5_6_5_REV;
		case pixel_type::UNSIGNED_SHORT_4_4_4_4:
			return GL_UNSIGNED_SHORT_4_4_4_4;
		case pixel_type::UNSIGNED_SHORT_4_4_4_4_REV:
			return GL_UNSIGNED_SHORT_4_4_4_4_REV;
		case pixel_type::UNSIGNED_SHORT_5_5_5_1:
			return GL_UNSIGNED_SHORT_5_5_5_1;
		case pixel_type::UNSIGNED_SHORT_1_5_5_5_REV:
			return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		case pixel_type::UNSIGNED_INT_8_8_8_8:
			return GL_UNSIGNED_INT_8_8_8_8;
		case pixel_type::UNSIGNED_INT_8_8_8_8_REV:
			return GL_UNSIGNED_INT_8_8_8_8_REV;
		case pixel_type::UNSIGNED_INT_10_10_10_2:
			return GL_UNSIGNED_INT_10_10_10_2;
		case pixel_type::UNSIGNED_INT_2_10_10_10_REV:
			return GL_UNSIGNED_INT_2_10_10_10_REV;
		default:
			throw std::exception();  // TODO specify
	}
}

GLenum ogl_cast(texture_wrap w)
{
	switch (w)
	{
		case texture_wrap::CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		case texture_wrap::CLAMP_TO_BORDER:
			return GL_CLAMP_TO_BORDER;
		case texture_wrap::MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		case texture_wrap::REPEAT:
			return GL_REPEAT;
		case texture_wrap::MIRROR_CLAMP_TO_EDGE:
			return GL_MIRROR_CLAMP_TO_EDGE;
		default:
			throw std::exception();  // TODO specify
	}
}

GLenum ogl_cast(texture_filter f)
{
	switch (f)
	{
		case texture_filter::NEAREST:
			return GL_NEAREST;
		case texture_filter::LINEAR:
			return GL_LINEAR;
		case texture_filter::NEAREST_MIPMAP_NEAREST:
			return GL_NEAREST_MIPMAP_NEAREST;
		case texture_filter::LINEAR_MIPMAP_NEAREST:
			return GL_LINEAR_MIPMAP_NEAREST;
		case texture_filter::NEAREST_MIPMAP_LINEAR:
			return GL_NEAREST_MIPMAP_LINEAR;
		case texture_filter::LINEAR_MIPMAP_LINEAR:
			return GL_LINEAR_MIPMAP_LINEAR;
		default:
			throw std::exception();  // TODO specify
	}
}

GLenum ogl_cast(compare_function cmp)
{
	switch (cmp)
	{
		case compare_function::LEQUAL:
			return GL_LEQUAL;
		case compare_function::GEQUAL:
			return GL_GEQUAL;
		case compare_function::LESS:
			return GL_LESS;
		case compare_function::GREATER:
			return GL_GREATER;
		case compare_function::EQUAL:
			return GL_EQUAL;
		case compare_function::NOTEQUAL:
			return GL_NOTEQUAL;
		case compare_function::ALWAYS:
			return GL_ALWAYS;
		case compare_function::NEVER:
			return GL_NEVER;
		default:
			throw std::exception();  // TODO specify
	}
}

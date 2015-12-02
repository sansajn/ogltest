#include "texture.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <GL/glew.h>

using std::max;
using std::string;
using std::unique_ptr;
using std::swap;
using std::logic_error;


static unsigned alignment_to(unsigned width, pixel_format pfmt, pixel_type type);
static unsigned pixel_sizeof(pixel_format pfmt, pixel_type type);
static unsigned channel_count(pixel_format pfmt);
static void deduce_pixel_format_and_type(sized_internal_format ifmt, pixel_format & pfmt, pixel_type & type);


texture::parameters::parameters()
	: _min{texture_filter::nearest}, _mag{texture_filter::linear}
{
	_wrap[0] = _wrap[1] = _wrap[2] = texture_wrap::clamp_to_edge;
}

texture::parameters & texture::parameters::min(texture_filter mode)
{
	_min = mode;
	return *this;
}

texture::parameters & texture::parameters::mag(texture_filter mode)
{
	_mag = mode;
	return *this;
}

texture::parameters & texture::parameters::filter(texture_filter minmag_mode)
{
	_min = _mag = minmag_mode;
	return *this;
}

texture::parameters & texture::parameters::filter(texture_filter min_mode, texture_filter mag_mode)
{
	_min = min_mode;
	_mag = mag_mode;
	return *this;
}

texture::parameters & texture::parameters::wrap_s(texture_wrap mode)
{
	_wrap[0] = mode;
	return *this;
}

texture::parameters & texture::parameters::wrap_t(texture_wrap mode)
{
	_wrap[1] = mode;
	return *this;
}

texture::parameters & texture::parameters::wrap_r(texture_wrap mode)
{
	_wrap[2] = mode;
	return *this;
}

texture::parameters & texture::parameters::wrap(texture_wrap mode)
{
	_wrap[0] = _wrap[1] = _wrap[2] = mode;
	return *this;
}


texture::texture(unsigned target, unsigned tid)
	: _tbo(tid), _target(target)
{
	assert(glIsTexture(tid) && "tid is not a texture id");
}

texture::~texture()
{
	glDeleteTextures(1, &_tbo);
}

void texture::bind(unsigned unit)
{
	assert(unit >= 0 && unit < 32 && "not enougth texture units");
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(_target, _tbo);
}

texture::texture(texture && lhs)
{
	_tbo = lhs._tbo;
	_target = lhs._target;
	lhs._tbo = 0;
}

void texture::operator=(texture && lhs)
{
	swap(_tbo, lhs._tbo);
	swap(_target, lhs._target);
}

void texture::init(parameters const & params)
{
	assert(!_tbo && "texture already created");

	glGenTextures(1, &_tbo);
	glBindTexture(_target, _tbo);

	glTexParameteri(_target, GL_TEXTURE_WRAP_S, opengl_cast(params.wrap_s()));
	glTexParameteri(_target, GL_TEXTURE_WRAP_T, opengl_cast(params.wrap_t()));
	glTexParameteri(_target, GL_TEXTURE_WRAP_R, opengl_cast(params.wrap_r()));

	glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, opengl_cast(params.min()));
	glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, opengl_cast(params.mag()));

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}


texture2d::texture2d() : _fid(0), _rid(0), _w(0), _h(0)
{}

texture2d::texture2d(unsigned width, unsigned height, sized_internal_format ifmt, parameters const & params)
	: texture(GL_TEXTURE_2D, params), _fid(0), _rid(0)
{
	_w = width;
	_h = height;
	deduce_pixel_format_and_type(ifmt, _fmt, _type);

	glTexStorage2D(GL_TEXTURE_2D, 1, opengl_cast(ifmt), _w, _h);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

texture2d::texture2d(unsigned tid, unsigned width, unsigned height, pixel_format pfmt, pixel_type type)
	: texture(GL_TEXTURE_2D, tid), _fid(0), _rid(0)
{
	_w = width;
	_h = height;
	_fmt = pfmt;
	_type = type;
}

texture2d::texture2d(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void const * pixels, parameters const & params)
	: texture(GL_TEXTURE_2D, params), _fid(0), _rid(0)
{
	read(width, height, ifmt, pfmt, type, pixels, params);
}

texture2d::~texture2d()
{
	glDeleteRenderbuffers(1, &_rid);
	glDeleteFramebuffers(1, &_fid);
}

void texture2d::read(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void const * pixels, parameters const & params)
{
	_w = width;
	_h = height;
	_fmt = pfmt;
	_type = type;

	bool mipmaps =
		params.min() == texture_filter::linear_mipmap_linear ||
		params.min() == texture_filter::linear_mipmap_nearest ||
		params.min() == texture_filter::nearest_mipmap_linear ||
		params.min() == texture_filter::nearest_mipmap_nearest;

	unsigned levels = mipmaps ? log2(max(_w, _h))+1 : 1;  // TODO: ma zmysel generovat tak detailne mipmapy ?

	glTexStorage2D(GL_TEXTURE_2D, levels, opengl_cast(ifmt), _w, _h);
	if (pixels)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment_to(_w, pfmt, type));
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _w, _h, opengl_cast(pfmt), opengl_cast(type), pixels);
		if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void texture2d::bind_as_render_target(bool depth)
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

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

texture2d::texture2d(texture2d && lhs) : texture(std::move(lhs))
{
	_fid = lhs._fid;
	_rid = lhs._rid;
	_w = lhs._w;
	_h = lhs._h;
	_fmt = lhs._fmt;
	_type = lhs._type;
	lhs._fid = lhs._rid = 0;
}

void texture2d::operator=(texture2d && lhs)
{
	texture::operator=(std::move(lhs));
	swap(_fid, lhs._fid);
	swap(_rid, lhs._rid);
	_w = lhs._w;
	_h = lhs._h;
	_fmt = lhs._fmt;
	_type = lhs._type;
}

void texture2d::create_framebuffer()
{
	glGenFramebuffers(1, &_fid);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fid);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id(), 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void texture2d::create_depthbuffer()
{
	glGenRenderbuffers(1, &_rid);
	glBindRenderbuffer(GL_RENDERBUFFER, _rid);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _w, _h);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rid);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

texture2d_array::texture2d_array(unsigned width, unsigned height, unsigned layers, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void * pixels, parameters const & params)
	: texture(GL_TEXTURE_2D_ARRAY, params)
{
	_w = width;
	_h = height;
	_l = layers;

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, opengl_cast(ifmt), _w, _h, _l);
	if (pixels)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, _w, _h, _l, opengl_cast(pfmt), opengl_cast(type), pixels);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

texture2d_array::texture2d_array(texture2d_array && lhs) : texture(std::move(lhs))
{
	_w = lhs._w;
	_h = lhs._h;
	_l = lhs._l;
}

void texture2d_array::operator=(texture2d_array && lhs)
{
	texture::operator=(std::move(lhs));
	_w = lhs._w;
	_h = lhs._h;
	_l = lhs._l;
}


unsigned opengl_cast(pixel_type t)
{
	switch (t)
	{
		case pixel_type::ub8: return GL_UNSIGNED_BYTE;
		case pixel_type::b8: return GL_BYTE;
		case pixel_type::us16: return GL_UNSIGNED_SHORT;
		case pixel_type::s16: return GL_SHORT;
		case pixel_type::ui32: return GL_UNSIGNED_INT;
		case pixel_type::i32: return GL_INT;
		case pixel_type::f32: return GL_FLOAT;
		case pixel_type::ub332: return GL_UNSIGNED_BYTE_3_3_2;
		case pixel_type::ub233r: return GL_UNSIGNED_BYTE_2_3_3_REV;
		case pixel_type::us565: return GL_UNSIGNED_SHORT_5_6_5;
		case pixel_type::us565r: return GL_UNSIGNED_SHORT_5_6_5_REV;
		case pixel_type::us4444: return GL_UNSIGNED_SHORT_4_4_4_4;
		case pixel_type::us4444r: return GL_UNSIGNED_SHORT_4_4_4_4_REV;
		case pixel_type::us5551: return GL_UNSIGNED_SHORT_5_5_5_1;
		case pixel_type::us1555r: return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		case pixel_type::ui8888: return GL_UNSIGNED_INT_8_8_8_8;
		case pixel_type::ui8888r: return GL_UNSIGNED_INT_8_8_8_8_REV;
		case pixel_type::ui1010102: return GL_UNSIGNED_INT_10_10_10_2;
		case pixel_type::ui2101010r: return GL_UNSIGNED_INT_2_10_10_10_REV;
		default:
			throw cast_error{"unknown pixel type"};
	}
}

unsigned opengl_cast(pixel_format f)
{
	switch (f)
	{
		case pixel_format::red: return GL_RED;
		case pixel_format::rg: return GL_RG;
		case pixel_format::rgb: return GL_RGB;
		case pixel_format::bgr: return GL_BGR;
		case pixel_format::rgba: return GL_RGBA;
		case pixel_format::bgra: return GL_BGRA;
		case pixel_format::redi: return GL_RED_INTEGER;
		case pixel_format::rgi: return GL_RG_INTEGER;
		case pixel_format::rgbi: return GL_RGB_INTEGER;
		case pixel_format::bgri: return GL_BGR_INTEGER;
		case pixel_format::rgbai: return GL_RGBA_INTEGER;
		case pixel_format::bgrai: return GL_BGRA_INTEGER;
		case pixel_format::stencil_index: return GL_STENCIL_INDEX;
		case pixel_format::depth_component: return GL_DEPTH_COMPONENT;
		case pixel_format::depth_stencil: return GL_DEPTH_STENCIL;
		default:
			throw cast_error{"unknow pixel format"};
	};
}

unsigned opengl_cast(sized_internal_format i)
{
	switch (i)
	{
		case sized_internal_format::r8: return GL_R8;
		case sized_internal_format::r8_snorm: return GL_R8_SNORM;
		case sized_internal_format::r16: return GL_R16;
		case sized_internal_format::r16_snorm: return GL_R16_SNORM;
		case sized_internal_format::rg8: return GL_RG8;
		case sized_internal_format::rg8_snorm: return GL_RG8_SNORM;
		case sized_internal_format::rg16: return GL_RG16;
		case sized_internal_format::rg16_snorm: return GL_RG16_SNORM;
		case sized_internal_format::r3g3b2: return GL_R3_G3_B2;
		case sized_internal_format::rgb4: return GL_RGB4;
		case sized_internal_format::rgb5: return GL_RGB5;
		case sized_internal_format::rgb8: return GL_RGB8;
		case sized_internal_format::rgb8_snorm: return GL_RGB8_SNORM;
		case sized_internal_format::rgb10: return GL_RGB10;
		case sized_internal_format::rgb12: return GL_RGB12;
		case sized_internal_format::rgb16_snorm: return GL_RGB16_SNORM;
		case sized_internal_format::rgba2: return GL_RGBA2;
		case sized_internal_format::rgba4: return GL_RGBA4;
		case sized_internal_format::rgb5_a1: return GL_RGB5_A1;
		case sized_internal_format::rgba8: return GL_RGBA8;
		case sized_internal_format::rgba8_snorm: return GL_RGBA8_SNORM;
		case sized_internal_format::rgb10_a2: return GL_RGB10_A2;
		case sized_internal_format::rgb10_a2ui: return GL_RGB10_A2UI;
		case sized_internal_format::rgba12: return GL_RGBA12;
		case sized_internal_format::rgba16: return GL_RGBA16;
		case sized_internal_format::srgb8: return GL_SRGB8;
		case sized_internal_format::srgb8_alpha8: return GL_SRGB8_ALPHA8;
		case sized_internal_format::r16f: return GL_R16F;
		case sized_internal_format::rg16f: return GL_RG16F;
		case sized_internal_format::rgb16f: return GL_RGB16F;
		case sized_internal_format::rgba16f: return GL_RGBA16F;
		case sized_internal_format::r32f: return GL_R32F;
		case sized_internal_format::rg32f: return GL_RG32F;
		case sized_internal_format::rgb32f: return GL_RGB32F;
		case sized_internal_format::rgba32f: return GL_RGBA32F;
		case sized_internal_format::r11f_g11f_b10f: return GL_R11F_G11F_B10F;
		case sized_internal_format::rgb9_e5: return GL_RGB9_E5;
		case sized_internal_format::r8i: return GL_R8I;
		case sized_internal_format::r8ui: return GL_R8UI;
		case sized_internal_format::r16i: return GL_R16I;
		case sized_internal_format::r16ui: return GL_R16UI;
		case sized_internal_format::rg32i: return GL_R32I;
		case sized_internal_format::rg32ui: return GL_R32UI;
		case sized_internal_format::rgb8i: return GL_RGB8I;
		case sized_internal_format::rgb8ui: return GL_RGB8UI;
		case sized_internal_format::rgb16i: return GL_RGB16I;
		case sized_internal_format::rgb16ui: return GL_RGB16UI;
		case sized_internal_format::rgb32i: return GL_RGB32I;
		case sized_internal_format::rgb32ui: return GL_RGB32UI;
		case sized_internal_format::rgba8i: return GL_RGBA8I;
		case sized_internal_format::rgba8ui: return GL_RGBA8UI;
		case sized_internal_format::rgba16i: return GL_RGBA16I;
		case sized_internal_format::rgba16ui: return GL_RGBA16UI;
		case sized_internal_format::rgba32i: return GL_RGBA32I;
		case sized_internal_format::rgba32ui: return GL_RGBA32UI;

		default:
			throw cast_error{"unknow sized internal format"};
	}
}

unsigned opengl_cast(texture_wrap w)
{
	switch (w)
	{
		case texture_wrap::clamp_to_edge: return GL_CLAMP_TO_EDGE;
		case texture_wrap::clamp_to_border: return GL_CLAMP_TO_BORDER;
		case texture_wrap::mirrored_repeat: return GL_MIRRORED_REPEAT;
		case texture_wrap::repeat: return GL_REPEAT;
		default:
			throw cast_error{"unknow texture wrap mode"};
	}
}

unsigned opengl_cast(texture_filter f)
{
	switch (f)
	{
		case texture_filter::nearest: return GL_NEAREST;
		case texture_filter::linear: return GL_LINEAR;
		case texture_filter::nearest_mipmap_nearest: return GL_NEAREST_MIPMAP_NEAREST;
		case texture_filter::linear_mipmap_nearest: return GL_LINEAR_MIPMAP_NEAREST;
		case texture_filter::nearest_mipmap_linear: return GL_NEAREST_MIPMAP_LINEAR;
		case texture_filter::linear_mipmap_linear: return GL_LINEAR_MIPMAP_LINEAR;
		default:
			throw cast_error{"unknow texture filter mode"};
	}
}

unsigned opengl_cast(internal_format i)
{
	switch (i)
	{
		case internal_format::depth_component: return GL_DEPTH_COMPONENT;
		case internal_format::depth_stencil: return GL_DEPTH_STENCIL;
		case internal_format::red: return GL_RED;
		case internal_format::rg: return GL_RG;
		case internal_format::rgb: return GL_RGB;
		case internal_format::rgba: return GL_RGBA;

		case internal_format::r8: return GL_R8;
		case internal_format::r8_snorm: return GL_R8_SNORM;
		case internal_format::r16: return GL_R16;
		case internal_format::r16_snorm: return GL_R16_SNORM;
		case internal_format::rg8: return GL_RG8;
		case internal_format::rg8_snorm: return GL_RG8_SNORM;
		case internal_format::rg16: return GL_RG16;
		case internal_format::rg16_snorm: return GL_RG16_SNORM;
		case internal_format::r3g3b2: return GL_R3_G3_B2;
		case internal_format::rgb4: return GL_RGB4;
		case internal_format::rgb5: return GL_RGB5;
		case internal_format::rgb8: return GL_RGB8;
		case internal_format::rgb8_snorm: return GL_RGB8_SNORM;
		case internal_format::rgb10: return GL_RGB10;
		case internal_format::rgb12: return GL_RGB12;
		case internal_format::rgb16_snorm: return GL_RGB16_SNORM;
		case internal_format::rgba2: return GL_RGBA2;
		case internal_format::rgba4: return GL_RGBA4;
		case internal_format::rgb5_a1: return GL_RGB5_A1;
		case internal_format::rgba8: return GL_RGBA8;
		case internal_format::rgba8_snorm: return GL_RGBA8_SNORM;
		case internal_format::rgb10_a2: return GL_RGB10_A2;
		case internal_format::rgb10_a2ui: return GL_RGB10_A2UI;
		case internal_format::rgba12: return GL_RGBA12;
		case internal_format::rgba16: return GL_RGBA16;
		case internal_format::srgb8: return GL_SRGB8;
		case internal_format::srgb8_alpha8: return GL_SRGB8_ALPHA8;
		case internal_format::r16f: return GL_R16F;
		case internal_format::rg16f: return GL_RG16F;
		case internal_format::rgb16f: return GL_RGB16F;
		case internal_format::rgba16f: return GL_RGBA16F;
		case internal_format::r32f: return GL_R32F;
		case internal_format::rg32f: return GL_RG32F;
		case internal_format::rgb32f: return GL_RGB32F;
		case internal_format::rgba32f: return GL_RGBA32F;
		case internal_format::r11f_g11f_b10f: return GL_R11F_G11F_B10F;
		case internal_format::rgb9_e5: return GL_RGB9_E5;
		case internal_format::r8i: return GL_R8I;
		case internal_format::r8ui: return GL_R8UI;
		case internal_format::r16i: return GL_R16I;
		case internal_format::r16ui: return GL_R16UI;
		case internal_format::rg32i: return GL_R32I;
		case internal_format::rg32ui: return GL_R32UI;
		case internal_format::rgb8i: return GL_RGB8I;
		case internal_format::rgb8ui: return GL_RGB8UI;
		case internal_format::rgb16i: return GL_RGB16I;
		case internal_format::rgb16ui: return GL_RGB16UI;
		case internal_format::rgb32i: return GL_RGB32I;
		case internal_format::rgb32ui: return GL_RGB32UI;
		case internal_format::rgba8i: return GL_RGBA8I;
		case internal_format::rgba8ui: return GL_RGBA8UI;
		case internal_format::rgba16i: return GL_RGBA16I;
		case internal_format::rgba16ui: return GL_RGBA16UI;
		case internal_format::rgba32i: return GL_RGBA32I;
		case internal_format::rgba32ui: return GL_RGBA32UI;

		case internal_format::compressed_red: return GL_COMPRESSED_RED;
		case internal_format::compressed_rg: return GL_COMPRESSED_RG;
		case internal_format::compressed_rgb: return GL_COMPRESSED_RGB;
		case internal_format::compressed_rgba: return GL_COMPRESSED_RGBA;
		case internal_format::compressed_srgb: return GL_COMPRESSED_SRGB;
		case internal_format::compressed_srgb_alpha: return GL_COMPRESSED_SRGB_ALPHA;
		case internal_format::compressed_red_rgtc1: return GL_COMPRESSED_RED_RGTC1;
		case internal_format::compressed_signed_red_rgtc1: return GL_COMPRESSED_SIGNED_RED_RGTC1;
		case internal_format::compressed_rg_rgtc2: return GL_COMPRESSED_RG_RGTC2;
		case internal_format::compressed_signed_rg_rgtc2: return GL_COMPRESSED_SIGNED_RG_RGTC2;
		case internal_format::compressed_rgba_bptc_unorm: return GL_COMPRESSED_RGBA_BPTC_UNORM;
		case internal_format::compressed_srgb_alpha_bptc_unorm: return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
		case internal_format::compressed_rgb_bptc_signed_float: return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
		case internal_format::compressed_rgb_bptc_unsigned_float: return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;

		default:
			throw cast_error{"unknow internal format"};
	}
}

unsigned alignment_to(unsigned width, pixel_format pfmt, pixel_type type)
{
	if ((width % 4) == 0)
		return 4;

	unsigned rowbytes = width * pixel_sizeof(pfmt, type);

	if ((rowbytes % 4) == 0)
		return 4;
	else if ((rowbytes % 2) == 0)
		return 2;
	else
		return 1;
}

unsigned channel_count(pixel_format pfmt)
{
	switch (pfmt)
	{
		case pixel_format::red:
		case pixel_format::redi:
		case pixel_format::stencil_index:
		case pixel_format::depth_component:
		case pixel_format::depth_stencil: return 1;

		case pixel_format::rg:
		case pixel_format::rgi:	return 2;

		case pixel_format::rgb:
		case pixel_format::bgr:
		case pixel_format::rgbi:
		case pixel_format::bgri: return 3;

		case pixel_format::rgba:
		case pixel_format::bgra:
		case pixel_format::rgbai:
		case pixel_format::bgrai: return 4;

		default:
			throw logic_error("unknown image pixel-format");
	}
}

unsigned pixel_sizeof(pixel_format pfmt, pixel_type type)
{
	switch (type)
	{
		case pixel_type::ub8:
		case pixel_type::b8:	return channel_count(pfmt);

		case pixel_type::us16:
		case pixel_type::s16: return 2*channel_count(pfmt);

		case pixel_type::ui32:
		case pixel_type::i32:
		case pixel_type::f32: return 4*channel_count(pfmt);

		case pixel_type::ub332:
		case pixel_type::ub233r: return 1;

		case pixel_type::us565:
		case pixel_type::us565r:
		case pixel_type::us4444:
		case pixel_type::us4444r:
		case pixel_type::us5551:
		case pixel_type::us1555r: return 2;

		case pixel_type::ui8888:
		case pixel_type::ui8888r:
		case pixel_type::ui1010102:
		case pixel_type::ui2101010r: return 4;

		default:
			throw logic_error{"unknown pixel_type"};
	}
}

void deduce_pixel_format_and_type(sized_internal_format ifmt, pixel_format & pfmt, pixel_type & type)
{
	switch (ifmt)
	{
		case sized_internal_format::r8:
			pfmt = pixel_format::red;
			type = pixel_type::ub8;
			return;

		case sized_internal_format::rgb8:
			pfmt = pixel_format::rgb;
			type = pixel_type::ub8;
			return;

		case sized_internal_format::rgba8:
			pfmt = pixel_format::rgba;
			type = pixel_type::ub8;
			return;

		// TODO: support more formats

		default:
			throw logic_error{"unable to deduce pixel format or type from internal texture format"};
	}
}

#include "texture.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>
#include <cassert>
#include <GL/glew.h>
#include <ImageMagick/Magick++.h>

using std::string;
using std::unique_ptr;
using std::swap;

static GLenum opengl_cast(pixel_type t);
static GLenum opengl_cast(pixel_format f);
static GLenum opengl_cast(internal_format i);
static string format_string(pixel_format f);
static unsigned pixel_type_size(pixel_type t);
static unsigned channels(pixel_format f);
static Magick::StorageType storage_type(pixel_type t);


texture::texture() : _tid(0), _fid(0), _rid(0), _w(0), _h(0)
{}

texture::texture(std::string const & fname) : _tid(0), _fid(0), _rid(0)
{
	read(fname);
}

texture::texture(unsigned width, unsigned height) : _tid(0), _fid(0), _rid(0)
{
	create(width, height);
}

texture::texture(unsigned tid, unsigned width, unsigned height, pixel_format pfmt, pixel_type type)
	: _tid(tid), _fid(0), _rid(0)
{
	assert(glIsTexture(tid) && "tid is not a texture id");

	_w = width;
	_h = height;
	_fmt = pfmt;
	_type = type;

	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

texture::texture(unsigned width, unsigned height, internal_format ifmt, pixel_format pfmt, pixel_type type, void * data)
	: _tid(0), _fid(0), _rid(0)
{
	read(width, height, ifmt, pfmt, type, data);
}

texture::texture(texture && lhs)
{
	_tid = lhs._tid;
	_fid = lhs._fid;
	_rid = lhs._rid;
	_w = lhs._w;
	_h = lhs._h;
	_fmt = lhs._fmt;
	_type = lhs._type;
	lhs._tid = lhs._fid = lhs._rid = 0;
}

texture::~texture()
{
	glDeleteTextures(1, &_tid);
	glDeleteRenderbuffers(1, &_rid);
	glDeleteFramebuffers(1, &_fid);
}

void texture::read(std::string const & fname)
{
	Magick::Image im(fname.c_str());
	im.flip();

	Magick::Blob imblob;  // TODO: treba ten obrazok kopirovat do blobu (nestaci blob s obrazku ?)
	im.write(&imblob, "RGBA");

	read(im.columns(), im.rows(), internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, (void *)imblob.data());
}

void texture::read(unsigned width, unsigned height, internal_format ifmt, pixel_format pfmt, pixel_type type, void * data)
{
	assert(!_tid && "rexture already created");

	_w = width;
	_h = height;
	_fmt = pfmt;
	_type = type;

	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexImage2D(GL_TEXTURE_2D, 0, opengl_cast(ifmt), width, height, 0, opengl_cast(pfmt), opengl_cast(type), data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture::write(std::string const & fname)
{
	unsigned ch = channels(_fmt);

	unsigned bsize = _w * _h * ch * pixel_type_size(_type);
	unique_ptr<uint8_t []> data(new uint8_t[bsize]);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glGetTexImage(GL_TEXTURE_2D, 0, opengl_cast(_fmt), opengl_cast(_type), data.get());

	Magick::Image im;
	im.read(_w, _h, format_string(_fmt), storage_type(_type), data.get());

	if (ch == 1)
		im.colorSpace(Magick::ColorspaceType::GRAYColorspace);
	else
		im.colorSpace(Magick::ColorspaceType::RGBColorspace);

	im.flip();
	im.write(fname);
}

void texture::create(unsigned width, unsigned height)
{
	assert(!_tid && "texture already created");

	_w = width;
	_h = height;
	_fmt = pixel_format::rgba;
	_type = pixel_type::ub8;

	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _w, _h);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture::bind(unsigned unit)
{
	assert(unit >= 0 && unit <= 31 && "not enougth texture units");  // TODO: zisti kolko mam texturovacich jednotiek
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, _tid);
}

void texture::bind_as_render_target(bool depth)
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
}

void texture::operator=(texture && lhs)
{
	swap(_tid, lhs._tid);
	swap(_fid, lhs._fid);
	swap(_rid, lhs._rid);
	_w = lhs._w;
	_h = lhs._h;
	_fmt = lhs._fmt;
	_type = lhs._type;
}

void texture::create_framebuffer()
{
	glGenFramebuffers(1, &_fid);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fid);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tid, 0);
}

void texture::create_depthbuffer()
{
	glGenRenderbuffers(1, &_rid);
	glBindRenderbuffer(GL_RENDERBUFFER, _rid);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _w, _h);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rid);
}

GLenum opengl_cast(pixel_type t)
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
			throw std::exception();  // unknown pixel type
	}
}

GLenum opengl_cast(pixel_format f)
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
			throw std::exception();  // unknown pixel format
	};
}

GLenum opengl_cast(internal_format i)
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
			throw std::exception();  // unknown internal-format
	}
}

string format_string(pixel_format f)
{
	switch (f)
	{
		case pixel_format::red:
		case pixel_format::redi:
		case pixel_format::stencil_index:
		case pixel_format::depth_component:
		case pixel_format::depth_stencil:
			return "I";

		case pixel_format::rg:
		case pixel_format::rgi:
			return "RG";

		case pixel_format::rgb:
		case pixel_format::rgbi:
			return "RGB";

		case pixel_format::bgr:
		case pixel_format::bgri:
			return "BGR";

		case pixel_format::rgba:
		case pixel_format::rgbai:
			return "RGBA";

		case pixel_format::bgra:
		case pixel_format::bgrai: return "BGRA";

		default:
			throw std::exception();  // TODO: specify
	}
}

unsigned pixel_type_size(pixel_type t)
{
	switch (t)
	{
		case pixel_type::ub8:
		case pixel_type::b8:
		case pixel_type::ub332:
		case pixel_type::ub233r:
			return 1;

		case pixel_type::us16:
		case pixel_type::s16:
		case pixel_type::us565:
		case pixel_type::us565r:
		case pixel_type::us4444:
		case pixel_type::us4444r:
		case pixel_type::us5551:
		case pixel_type::us1555r:
			return 2;

		case pixel_type::ui32:
		case pixel_type::i32:
		case pixel_type::f32:
		case pixel_type::ui8888:
		case pixel_type::ui8888r:
		case pixel_type::ui1010102:
		case pixel_type::ui2101010r:
			return 4;

		default:
			throw std::exception();  // TODO: specify
	}
}

unsigned channels(pixel_format f)
{
	switch (f)
	{
		case pixel_format::red:
		case pixel_format::redi:
		case pixel_format::stencil_index:
		case pixel_format::depth_component:
		case pixel_format::depth_stencil:
			return 1;

		case pixel_format::rg:
		case pixel_format::rgi:
			return 2;

		case pixel_format::rgb:
		case pixel_format::bgr:
		case pixel_format::rgbi:
		case pixel_format::bgri:
			return 3;

		case pixel_format::rgba:
		case pixel_format::bgra:
		case pixel_format::rgbai:
		case pixel_format::bgrai:
			return 4;

		default:
			throw std::exception();  // TODO: specify
	}
}

Magick::StorageType storage_type(pixel_type t)
{
	switch (t)
	{
		case pixel_type::ub8:
		case pixel_type::b8:
		case pixel_type::ub332:
		case pixel_type::ub233r:
			return Magick::StorageType::CharPixel;

		case pixel_type::us16:
		case pixel_type::s16:
		case pixel_type::us565:
		case pixel_type::us565r:
		case pixel_type::us4444:
		case pixel_type::us4444r:
		case pixel_type::us5551:
		case pixel_type::us1555r:
			return Magick::StorageType::ShortPixel;

		case pixel_type::ui32:
		case pixel_type::i32:
		case pixel_type::ui8888:
		case pixel_type::ui8888r:
		case pixel_type::ui1010102:
		case pixel_type::ui2101010r:
			return Magick::StorageType::IntegerPixel;

		case pixel_type::f32: return Magick::StorageType::FloatPixel;

		default:
			throw std::exception();  // TODO: specify
	}
}

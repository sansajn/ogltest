#include "texture_loader.hpp"
#include <memory>
#include <string>
#include <Magick++.h>
#include <GL/glew.h>

using std::unique_ptr;
using std::string;

namespace gl {

static unsigned channels(pixel_format f);
static unsigned depth(pixel_type t);
static string format_string(pixel_format f);
static Magick::StorageType storage_type(pixel_type t);


texture2d texture_from_file(std::string const & fname, texture::parameters const & params)
{
	Magick::Image im(fname);
	im.flip();

	Magick::Blob imblob;
	im.write(&imblob, "RGBA");

	return texture2d{im.columns(), im.rows(), sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, imblob.data(), params};
}


void texture_write_to_file(texture2d const & t, std::string const & fname)
{
	pixel_type ptype = t.pixtype();
	pixel_format pfmt = t.pixfmt();
	unsigned ch = channels(pfmt);
	unsigned w = t.width(), h = t.height();

	unsigned bsize = w * w * ch * depth(ptype);
	unique_ptr<uint8_t []> pixels{new uint8_t[bsize]};
	glBindTexture(GL_TEXTURE_2D, t.id());
	glGetTexImage(GL_TEXTURE_2D, 0, opengl_cast(pfmt), opengl_cast(ptype), pixels.get());

	Magick::Image im;
	im.read(w, h, format_string(pfmt), storage_type(ptype), pixels.get());

	if (ch == 1)
		im.colorSpace(Magick::ColorspaceType::GRAYColorspace);
	else
		im.colorSpace(Magick::ColorspaceType::RGBColorspace);

	im.flip();
	im.write(fname);
}

void texture_write_to_file(texture2d_array const & t, unsigned layer, std::string const & fname)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, t.id());

	int ifmt;
	glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT, &ifmt);

	pixel_type type;
	pixel_format format;

	switch (ifmt)
	{
		case GL_RGBA8:
			type = pixel_type::ub8;
			format = pixel_format::rgba;
			break;

		case GL_R16F:
		case GL_R32F:
			type = pixel_type::f32;
			format = pixel_format::red;
			break;

		default:
			throw cast_error{"unsupported texture format"};
	}

	unsigned ch = channels(format);
	unsigned w = t.width(), h = t.height();
	unsigned size = w*h*ch*depth(type);
	unique_ptr<uint8_t []> data(new uint8_t[size*t.layers()]);
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, opengl_cast(format), opengl_cast(type), data.get());

	Magick::Image im;
	uint8_t * pixels = data.get() + size*layer;
	im.read(w, h, format_string(format), storage_type(type), pixels);

	if (ch == 1)
		im.colorSpace(Magick::ColorspaceType::GRAYColorspace);
	else
		im.colorSpace(Magick::ColorspaceType::RGBColorspace);

	im.flip();
	im.write(fname);
}


unsigned channels(pixel_format f)
{
	switch (f)
	{
		case pixel_format::red:
		case pixel_format::redi:
		case pixel_format::stencil_index:
		case pixel_format::depth_component:
		case pixel_format::depth_stencil: return 1;

		case pixel_format::rg:
		case pixel_format::rgi: return 2;

		case pixel_format::rgb:
		case pixel_format::bgr:
		case pixel_format::rgbi:
		case pixel_format::bgri: return 3;

		case pixel_format::rgba:
		case pixel_format::bgra:
		case pixel_format::rgbai:
		case pixel_format::bgrai: return 4;

		default:
			throw cast_error{"unknown pixel-format"};
	}
}

unsigned depth(pixel_type t)
{
	switch (t)
	{
		case pixel_type::ub8:
		case pixel_type::b8:
		case pixel_type::ub332:
		case pixel_type::ub233r: return 1;

		case pixel_type::us16:
		case pixel_type::s16:
		case pixel_type::us565:
		case pixel_type::us565r:
		case pixel_type::us4444:
		case pixel_type::us4444r:
		case pixel_type::us5551:
		case pixel_type::us1555r: return 2;

		case pixel_type::ui32:
		case pixel_type::i32:
		case pixel_type::f32:
		case pixel_type::ui8888:
		case pixel_type::ui8888r:
		case pixel_type::ui1010102:
		case pixel_type::ui2101010r: return 4;

		default:
			throw cast_error{"unknown pixel-type"};
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
		case pixel_format::depth_stencil: return "I";

		case pixel_format::rg:
		case pixel_format::rgi: return "RG";

		case pixel_format::rgb:
		case pixel_format::rgbi: return "RGB";

		case pixel_format::bgr:
		case pixel_format::bgri: return "BGR";

		case pixel_format::rgba:
		case pixel_format::rgbai: return "RGBA";

		case pixel_format::bgra:
		case pixel_format::bgrai: return "BGRA";

		default:
			throw cast_error{"unknown pixel-format"};
	}
}

Magick::StorageType storage_type(pixel_type t)
{
	switch (t)
	{
		case pixel_type::ub8:
		case pixel_type::b8:
		case pixel_type::ub332:
		case pixel_type::ub233r: return Magick::StorageType::CharPixel;

		case pixel_type::us16:
		case pixel_type::s16:
		case pixel_type::us565:
		case pixel_type::us565r:
		case pixel_type::us4444:
		case pixel_type::us4444r:
		case pixel_type::us5551:
		case pixel_type::us1555r: return Magick::StorageType::ShortPixel;

		case pixel_type::ui32:
		case pixel_type::i32:
		case pixel_type::ui8888:
		case pixel_type::ui8888r:
		case pixel_type::ui1010102:
		case pixel_type::ui2101010r: return Magick::StorageType::IntegerPixel;

		case pixel_type::f32: return Magick::StorageType::FloatPixel;

		default:
			throw cast_error{"unknown pixel-type"};
	}
}

}  // gl

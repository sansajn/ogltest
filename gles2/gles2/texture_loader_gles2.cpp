#include "texture_loader_gles2.hpp"
#include <GL/glew.h>
#include "pix/pix_png.hpp"
#include "pix/pix_jpeg.hpp"

namespace gles2 {

using std::string;

static pixel_format match_pixel_format(uint8_t channels);
static pixel_type match_pixel_type(uint8_t depth);
static string extension(string const & path);


gles2::texture2d texture_from_file(std::string const & fname, texture::parameters const & params)
{
	string ext = extension(fname);
	if (ext == "jpg" || ext == "jpeg")
	{
		pix::jpeg_decoder d;
		d.decode(fname);
		pix::flip(d.result.height, d.result.rowbytes, d.result.pixels);
		return texture2d{d.result.width, d.result.height, match_pixel_format(d.result.channels), match_pixel_type(d.result.depth), d.result.pixels, params};
	}
	else if (ext == "png")
	{
		pix::png_decoder d;
		d.decode(fname);
		pix::flip(d.result.height, d.result.rowbytes, d.result.pixels);
		return gles2::texture2d{d.result.width, d.result.height, gles2::pixel_format::rgba, gles2::pixel_type::ub8, d.result.pixels, params};
	}
	else
	{
		// TODO: ak sa nepodari podla koncovky, skus podla magick-u (prvych n bajtov subora)
		throw std::logic_error{"unknown image type"};
	}
}


pixel_format match_pixel_format(uint8_t channels)
{
	switch (channels)
	{
		case 3: return pixel_format::rgb;
		case 4: return pixel_format::rgba;
		case 1: return pixel_format::luminance;
		default:
			throw std::logic_error{"unable to match a pixel-format"};
	}
}

pixel_type match_pixel_type(uint8_t depth)
{
	switch (depth)
	{
		case 1: return pixel_type::ub8;
		default:
			throw std::logic_error{"unable to match a pixel-type"};
	}
}

string extension(string const & path)
{
	auto dot_pos = path.rfind('.');
	if (dot_pos != string::npos && dot_pos+1 < path.length())
		return string{path.begin() + (dot_pos+1), path.end()};
	else
		return string{};
}

}  // gles2

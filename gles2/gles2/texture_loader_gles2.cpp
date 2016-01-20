#include "texture_loader_gles2.hpp"
#include <GL/glew.h>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>

namespace gles2 {

using std::string;
using gles2::texture2d;

static string extension(string const & path);


texture2d texture_from_file(std::string const & fname, texture::parameters const & params)
{
	using namespace boost::gil;

	rgba8_image_t im;

	string ext = extension(fname);
	if (ext == "jpg" || ext == "jpeg")
		jpeg_read_and_convert_image(fname, im);
	else if (ext == "png")
		png_read_and_convert_image(fname, im);
	else
		throw std::logic_error{"unknown image type"};  // TODO: ak sa nepodari podla koncovky, skus podla magick-u (prvych n bajtov subora)

	assert(im.width() > 0 && im.height() > 0 && "invalid image dimension");

	rgba8_image_t flipped_im{im.dimensions()};
	copy_pixels(view(im), flipped_up_down_view(view(flipped_im)));

	void * pixels = (void *)&(*view(flipped_im).begin());
	return texture2d(im.width(), im.height(), gles2::pixel_format::rgba, gles2::pixel_type::ub8, pixels, params);
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

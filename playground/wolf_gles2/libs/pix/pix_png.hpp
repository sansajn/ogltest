#pragma once
#include <string>
#include <boost/gil/gil_all.hpp>
#include "pix.hpp"

namespace pix {

//! PNG image decoder implementation (based on libpng12).
struct png_decoder : public decoder
{
	//! \note allways decode pixels as rgba8
	void decode(std::string const & fname) override;
};

boost::gil::rgba8_view_t png_view_from_file(char const * fname);
boost::gil::rgba8_view_t png_view_from_file(std::string const & fname);

}  // pix

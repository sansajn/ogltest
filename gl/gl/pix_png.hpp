#pragma once
#include <string>
#include <cstdint>
#include "pix.hpp"

namespace pix {

//! PNG images decoder implementation (based on libpng12).
struct png_decoder : public decoder
{
	void decode_as_rgba8(std::string const & fname) override;
};

}  // pix

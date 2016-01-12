#pragma once
#include <string>
#include <cstdint>
#include "pix.hpp"

namespace pix {

//! PNG image decoder implementation (based on libpng12).
struct png_decoder : public decoder
{
	void decode(std::string const & fname) override;
};

}  // pix

#pragma once
#include "pix.hpp"

namespace pix {

//! JPEG image decoder implementation (based on libjpeg-v8c).
struct jpeg_decoder : public decoder
{
	void decode(std::string const & fname) override;
};

}  // pix

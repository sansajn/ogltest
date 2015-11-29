#pragma once
#include <string>
#include "gl/gles2/texture_gles2.hpp"

namespace gl {
	namespace gles2 {

gles2::texture2d texture_from_file(std::string const & fname);

	}  // gles2
}  // gl

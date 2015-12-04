#pragma once
#include <string>
#include "gl/gles2/texture_gles2.hpp"

namespace gles2 {

texture2d texture_from_file(std::string const & fname, texture::parameters const & params = texture::parameters{});

}  // gles2

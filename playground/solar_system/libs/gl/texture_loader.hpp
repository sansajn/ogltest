// citanie textur zo suborov
#pragma once
#include <string>
#include "gl/texture.hpp"

namespace gl {

texture2d texture_from_file(std::string const & fname, texture::parameters const & params = texture::parameters{});

void texture_write_to_file(texture2d const & t, std::string const & fname);
void texture_write_to_file(texture2d_array const & t, unsigned layer, std::string const & fname);

}  // gl

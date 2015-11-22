#pragma once
#include "mesh.hpp"
#include "model.hpp"

namespace gl {

mesh mesh_from_file(std::string const & fname, unsigned mesh_idx = 0);
mesh mesh_from_memory(void const * buf, unsigned len, char const * file_format);

model model_from_file(std::string const & fname);

}  // gl

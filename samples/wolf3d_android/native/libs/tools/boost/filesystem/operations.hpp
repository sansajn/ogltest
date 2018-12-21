#pragma once
#include "path.hpp"
#include <cstdint>

namespace boost { namespace filesystem {

bool exists(path const & p);
uintmax_t file_size(path const & p);

}}  // boost::filesystem

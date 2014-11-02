#pragma once
#include <GL/glew.h>
#include "render/detail/texture_types.hpp"

GLenum ogl_cast(texture_internal_format fi);
GLenum ogl_cast(texture_format f);
GLenum ogl_cast(pixel_type t);
GLenum ogl_cast(texture_wrap w);
GLenum ogl_cast(texture_filter f);
GLenum ogl_cast(compare_function cmp);

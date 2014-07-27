#pragma once
#include <GL/glew.h>
#include "types.h"

GLenum ogl_cast(mesh_mode m);
GLenum ogl_cast(polygon_rasterization r);
GLenum ogl_cast(attribute_type t);
GLenum ogl_cast(buffer_usage u);

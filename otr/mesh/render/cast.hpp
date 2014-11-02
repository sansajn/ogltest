#pragma once
#include <GL/glew.h>
#include "render/mesh_types.hpp"

GLenum ogl_cast(mesh_mode m);
GLenum ogl_cast(attribute_type t);
GLenum ogl_cast(polygon_raster_mode r);

#include "render/detail/buffer_cast.hpp"
#include <exception>

GLenum ogl_cast(buffer_usage u)
{
	switch (u)
	{
		case buffer_usage::STREAM_DRAW:
			return GL_STREAM_DRAW;
		case buffer_usage::STREAM_READ:
			return GL_STREAM_READ;
		case buffer_usage::STREAM_COPY:
			return GL_STREAM_COPY;
		case buffer_usage::STATIC_DRAW:
			return GL_STATIC_DRAW;
		case buffer_usage::STATIC_READ:
			return GL_STATIC_READ;
		case buffer_usage::STATIC_COPY:
			return GL_STATIC_COPY;
		case buffer_usage::DYNAMIC_DRAW:
			return GL_DYNAMIC_DRAW;
		case buffer_usage::DYNAMIC_READ:
			return GL_DYNAMIC_READ;
		case buffer_usage::DYNAMIC_COPY:
			return GL_DYNAMIC_COPY;
		default:
			throw std::exception();  // TODO specify (unknown buffer usage)
	}  // switch
}

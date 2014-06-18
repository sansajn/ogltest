#include "framebuffer.h"

frame_buffer & frame_buffer::default_fb()
{
	static frame_buffer fb;
	return fb;
}

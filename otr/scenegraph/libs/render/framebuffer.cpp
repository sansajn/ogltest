#include "framebuffer.h"

frame_buffer & frame_buffer::default_fb()
{
	static frame_buffer fb;
	return fb;
}

void frame_buffer::depth_test(bool enable)
{
	if (enable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void frame_buffer::clear(bool color, bool depth, bool stencil)
{
	int buffers = 0;
	if (color)
		buffers |= GL_COLOR_BUFFER_BIT;	
	if (depth)
		buffers |= GL_DEPTH_BUFFER_BIT;
	if (stencil)
		buffers |= GL_STENCIL_BUFFER_BIT;

	glClear(buffers);
}

void frame_buffer::polygon_mode(GLenum mode)
{
	glPolygonMode(GL_FRONT_AND_BACK, mode);
}

void frame_buffer::draw(shader_program const & p, mesh_buffers const & m)
{
	p.use();
	m.draw();
}

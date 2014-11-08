#include "render/framebuffer.hpp"
#include "render/cast.hpp"

framebuffer & framebuffer::default_fb()
{
	static framebuffer fb;
	return fb;
}

void framebuffer::depth_test(bool enable)
{
	if (enable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void framebuffer::clear(bool color, bool depth, bool stencil)
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

void framebuffer::polygon_mode(polygon_raster_mode r)
{
	glPolygonMode(GL_FRONT_AND_BACK, ogl_cast(r));
}

void framebuffer::draw(shader::program & p, mesh_buffers const & m)
{
	p.set();
	m.draw();
}

#pragma once
#include "mesh.h"
#include "meshbuffers.h"
#include "program.h"

/*! zovseobecnenie opengl contextu a spojenie s vykreslitelnými objektami */
class frame_buffer
{
public:
	static frame_buffer & default_fb();

	void depth_test(bool enable);
	void clear(bool color, bool depth, bool stencil = false);

	template <typename Vertex, typename Index>
	void draw(gl::program const & p, mesh<Vertex, Index> const & m);

	void draw(gl::program const & p, mesh_buffers const & m, GLenum mode);
};


template <typename Vertex, typename Index>
void frame_buffer::draw(gl::program const & p, mesh<Vertex, Index> const & m)
{
	draw(p, *m.buf(), m.mode());
}

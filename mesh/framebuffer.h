#pragma once
#include "mesh.h"
#include "program.h"

/*! spojenie s opengl kontextom a vykreslitetilnými objektami */
class frame_buffer
{
public:
	static frame_buffer & default_fb();

	template <typename Vertex, typename Index>
	void draw(gl::program const & p, mesh<Vertex, Index> const & mesh);
};


template <typename Vertex, typename Index>
void frame_buffer::draw(gl::program const & p, mesh<Vertex, Index> const & m)
{
	p.use();
	m.buf()->draw(m.mode(),
		m.indice_count() == 0 ? m.vertex_count() : m.indice_count());
}

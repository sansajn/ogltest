#pragma once
#include <glm/vec4.hpp>
#include "render/mesh.h"
#include "render/meshbuffers.h"
#include "render/program.h"

/*! zovseobecnenie opengl contextu a spojenie s vykreslitelnými objektami
@ingroup render */
class frame_buffer
{
public:
	static frame_buffer & default_fb();

	void depth_test(bool enable);
	void clear(bool color, bool depth, bool stencil = false);
	void polygon_mode(GLenum mode);  // TODO: change gl-enum to real enum
	glm::ivec4 viewport() const {return glm::ivec4(0, 0, 0, 0);}  // TODO: fake implementation

	template <typename Vertex, typename Index>
	void draw(shader_program const & p, mesh<Vertex, Index> const & m);

	void draw(shader_program const & p, mesh_buffers const & m);
};


template <typename Vertex, typename Index>
void frame_buffer::draw(shader_program const & p, mesh<Vertex, Index> const & m)
{
	draw(p, *m.buf());
}

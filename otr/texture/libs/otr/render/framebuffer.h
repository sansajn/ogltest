#pragma once
#include <glm/vec4.hpp>
#include "render/mesh.h"
#include "render/meshbuffers.h"
#include "render/program.h"
#include "render/types.h"

/*! zovseobecnenie opengl contextu a spojenie s vykreslitelnými objektami
@ingroup render */
class framebuffer
{
public:
	static framebuffer & default_fb();

	void depth_test(bool enable);
	void clear(bool color, bool depth, bool stencil = false);
	void polygon_mode(polygon_raster_mode r);
	glm::ivec4 viewport() const {return glm::ivec4(0, 0, 0, 0);}  // TODO: fake implementation
	void viewport(glm::ivec4 const & v) {}  // TODO: fake implementation

	template <typename Vertex, typename Index>
	void draw(shader_program const & p, mesh<Vertex, Index> const & m);

	void draw(shader_program const & p, mesh_buffers const & m);
};


template <typename Vertex, typename Index>
void framebuffer::draw(shader_program const & p, mesh<Vertex, Index> const & m)
{
	draw(p, *m.buf());
}

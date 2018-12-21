#include "debug_draw_gles2.hpp"
#include <string>
#include "gles2/mesh_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"

using std::string;
using glm::vec3;
using gles2::mesh;

namespace phys {

debug_draw_impl::debug_draw_impl()
	: _mode{DBG_NoDebug}
{
	_solid.from_memory(gles2::flat_shader_source);
}

void debug_draw_impl::update(glm::mat4 const & world_to_screen)
{
	_world_to_screen = world_to_screen;
}

void debug_draw_impl::drawContactPoint(btVector3 const & pointOnB, btVector3 const & normalOnB,
	btScalar distance, int lifeTime, btVector3 const & color)
{
	btVector3 const startPoint = pointOnB;
	btVector3 const endPoint = startPoint + normalOnB * distance;
	drawLine(startPoint, endPoint, color);
}

static mesh make_line_dmesh(gles2::shader::program const & prog)
{
	mesh m{2*3*sizeof(float), 2, gles2::buffer_usage::dynamic_draw};
	m.append_attribute(gles2::attribute(prog.attribute_location("position"), 3, GL_FLOAT, 3*sizeof(GLfloat), 0));
	m.draw_mode(gles2::render_primitive::lines);
	return m;
}

void debug_draw_impl::drawLine(btVector3 const & from, btVector3 const & to, btVector3 const & color)
{
	static mesh m = make_line_dmesh(_solid);  // TODO: mega pomala implementacia

	float verts[] = {from[0], from[1], from[2], to[0], to[1], to[2]};  // position:3
	unsigned indices[] = {0,1};

	m.data(verts, sizeof(verts), indices, 2);

	_solid.use();
	_solid.uniform_variable("local_to_screen", _world_to_screen);
	_solid.uniform_variable("color", vec3{color[0], color[1], color[2]});

	m.render();
}

void debug_draw_impl::toggle_debug_flag(int flag)
{
	if (_mode & flag)
		_mode = _mode & (~flag);  // disable flag
	else
		_mode |= flag;  // enble flag
}

}  // phys


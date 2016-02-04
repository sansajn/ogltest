#include "debug_draw.hpp"
#include <string>
#include "gl/mesh.hpp"

using std::string;
using glm::vec3;
using gl::mesh;

namespace phys {

static string solid_shader_source = R"(
	// zobrazi model vo farbe, bez osvetlenia
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	void main()	{
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main() {
		fcolor = vec4(color, 1);
	}
	#endif
)";


debug_draw_impl::debug_draw_impl()
	: _mode{DBG_NoDebug}
{
	_solid.from_memory(solid_shader_source);
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

static mesh make_line_dmesh()
{
	mesh m{2*3*sizeof(float), 2, gl::buffer_usage::dynamic_draw};
	m.attach_attributes({
		mesh::vertex_attribute_type{0, 3, GL_FLOAT, 3*sizeof(GLfloat), 0}
	});
	m.draw_mode(gl::render_primitive::lines);
	return m;
}

void debug_draw_impl::drawLine(btVector3 const & from, btVector3 const & to, btVector3 const & color)
{
	static mesh m = make_line_dmesh();  // TODO: mega pomala implementacia

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


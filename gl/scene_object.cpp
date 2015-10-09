#include "scene_object.hpp"
#include <string>
#include <glm/gtx/transform.hpp>
#include "mesh.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::translate;
using glm::scale;
using gl::mesh;
using gl::make_axis;
using gl::make_sphere;

static string colored_shader_source = R"(
	// zozbrazi farebny model
	uniform mat4 local_to_screen;
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 color;
	out vec3 vcolor;
	void main() {
		vcolor = color;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 vcolor;
	out vec4 fcolor;
	void main() {
		fcolor = vec4(vcolor, 1);
	}
	#endif
)";

static string solid_shader_source = R"(
	// zobrazi model bez osvetlenia
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
	void main()	{
		fcolor = vec4(color, 1);
	}
	#endif
)";

axis_object::axis_object()
{
	_prog.from_memory(colored_shader_source);
}

void axis_object::render(gl::camera & c)
{
	static mesh m = make_axis();
	_prog.use();
	_prog.uniform_variable("local_to_screen", c.view_projection());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	m.render();
}

light_object::light_object(glm::vec3 const & color)
	: _color{color}
{
	_prog.from_memory(solid_shader_source);
}

void light_object::render(gl::camera const & c, glm::vec3 const & position)
{
	static mesh m = make_sphere();
	mat4 M = translate(position) * scale(vec3{0.1});
	_prog.use();
	_prog.uniform_variable("local_to_screen", c.view_projection() * M);
	_prog.uniform_variable("color", _color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	m.render();
}

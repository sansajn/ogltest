// dynamicka mriezka
#include <algorithm>
#include <utility>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "mesh.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "window.hpp"
#include "scene_object.hpp"
#include "colors.hpp"

using std::swap;
using std::vector;
using std::string;
using std::logic_error;
using glm::vec3;
using glm::radians;
using gl::attribute;
using gl::mesh;
using gl::buffer_usage;
using gl::buffer_target;
using gl::camera;
using gl::free_look;
using gl::free_move;
using gl::render_primitive;
using ui::glut_pool_window;

string solid_shader_source = R"(
	// zobrazi model bez osvetlenia v zakladnej farbe
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

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	gl::mesh _line;

	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;
	shader::program _prog;
	axis_object _axis;
};

mesh make_line_dmesh(vec3 const & from, vec3 const & to)
{
	float verts[] = {from.x, from.y, from.z, to.x, to.y, to.z};  // position:3
	unsigned indices[] = {0,1};
	mesh m{verts, sizeof(verts), indices, 2, buffer_usage::stream_draw};
	m.append_attribute(attribute{0, 3, GL_FLOAT, 3*sizeof(float), 0});
	m.draw_mode(render_primitive::lines);
	return m;
}

scene_window::scene_window() : _look{_cam, *this}, _move{_cam, *this}
{
	_line = make_line_dmesh(vec3{0,0,0}, vec3{1,1,1});
	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_prog.from_memory(solid_shader_source);
}

void scene_window::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

void scene_window::update(float dt)
{
	base::update(dt);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_prog.use();
	_prog.uniform_variable("local_to_screen", _cam.view_projection());
	_prog.uniform_variable("color", rgb::yellow);
	_line.render();
	_axis.render(_cam);
	base::display();
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

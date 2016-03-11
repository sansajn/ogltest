// mesh v line-strip mode (standardne triangle)
#include <vector>
#include "gl/glut_window.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/shapes.hpp"

using std::vector;
using namespace gl;

char const * view_shader_source = R"(
	uniform mat4 local_to_screen;
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	void main() {
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main() {
		fcolor = vec4(1,1,1,1);
	}
	#endif
)";

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	mesh _plane;
	mesh _path;
	shader::program _prog;
	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;
};

scene_window::scene_window() : _look(_cam, *this), _move(_cam, *this)
{
	double size = 1.0;
	_plane = make_quad_xy<mesh>(glm::vec2{-size, -size}, 2*size);

	// [path]
	int nverts = 51;
	vector<vertex> verts;
	for (int i = 0; i < nverts; ++i)
	{
		float x = (i-(nverts/2)) * 2.0/(nverts-1.0);
		float y = x*x * (x < 0 ? -1 : 1);
		verts.emplace_back(glm::vec3{x,y,0}, glm::vec3{0,0,1});
	}
	_path = mesh_from_vertices(verts, render_primitive::line_strip);

	_prog.from_memory(view_shader_source);
	_cam = camera{glm::vec3{0,0,5}, glm::radians(70.0f), aspect_ratio(), 0.001, 1000};

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_prog.use();
	_prog.uniform_variable("local_to_screen", _cam.world_to_screen());
	_plane.render();
	_path.render();
	base::display();
}

void scene_window::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

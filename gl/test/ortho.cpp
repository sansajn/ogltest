#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/gl.hpp"
#include "gl/shapes.hpp"

using glm::mat4;
using glm::vec3;
using glm::mat3;
using glm::translate;
using glm::scale;
using glm::inverseTranspose;

char const * shaded_shader_source = R"(
	// zozbrazi model s tienovanim
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,2,3));
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main() {
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	void main() {
		fcolor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
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
	gl::mesh _cube;
	shader::program _shaded;
	axis_object _axis;
	gl::camera _cam;
	gl::free_look<scene_window> _look;
	gl::free_move<scene_window> _move;
};

scene_window::scene_window()
	: _cam{glm::ortho(0.0f, (float)width(), (float)height(), 0.0f, 0.01f, 1000.0f)}
	, _look{_cam, *this}
	, _move{_cam, *this}
{
	_cube = gl::make_cube<gl::mesh>();
	_shaded.from_memory(shaded_shader_source);
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	mat4 world_to_screen = _cam.world_to_screen();
	mat4 M = translate(vec3{400,300,-50}) * scale(vec3{100});

	_shaded.use();
	_shaded.uniform_variable("local_to_screen", world_to_screen * M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::lime);
	_cube.render();

	_axis.render(world_to_screen);
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

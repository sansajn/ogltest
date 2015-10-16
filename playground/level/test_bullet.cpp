// implementuje debug drawer a physics object
#include <algorithm>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "window.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "scene_object.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "physics.hpp"
#include "physics_debug.hpp"

using std::pair;
using std::make_pair;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::cout;
using std::swap;
using std::vector;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::inverseTranspose;
using glm::normalize;
using glm::scale;
using gl::mesh;
using gl::make_quad_xy;
using gl::make_quad_xz;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;

string shaded_shader_source = R"(
	// zozbrazi model s tienovanim zalozenom na normale vrchola
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,1,1));
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main()	{
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	void main()	{
		vec3 vcolor;
		if (gl_FrontFacing)
			vcolor = vec3(0,1,0);
		else
			vcolor = vec3(1,0,0);
		fcolor = vec4(max(dot(n, light_dir), 0.2) * vcolor, 1);
//		fcolor = vec4(color, 1);
	}
	#endif
)";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	~scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	// physics
	debug_drawer _ddraw;
	physics_object _cube;
	rigid_body_world _world;

	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;
	axis_object _axis;
	GLuint _vao;
};

scene_window::scene_window()
	: _look{_cam, *this}
	, _move{_cam, *this}
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.position = vec3{0, 5, 10};

	auto shape = shared_ptr<btCollisionShape>{new btBoxShape{btVector3{0.5, 0.5, 0.5}}};
	_cube = physics_object{shape};

	_world.add(_cube.body());
	_world.debug_drawer(&_ddraw);

	glClearColor(0, 0, 0, 1);
}

scene_window::~scene_window()
{
//	delete _shape;
	glDeleteVertexArrays(1, &_vao);
}

void scene_window::input(float dt)
{
	if (in.key_up('g'))
		_ddraw.toggle_debug_flag(btIDebugDraw::DBG_DrawWireframe);

	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_world.simulate(dt);
	_ddraw.update(_cam);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_world.debug_draw();
	_axis.render(_cam);
	base::display();
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

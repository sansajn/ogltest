// hod kockou na stenu
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
using std::string;
using std::vector;
using std::cout;
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
	void fire();

	mesh _wall;
	mesh _plane;
	shader::program _prog;
	axis_object _axis;
	light_object _light;
	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;

	// physics
	rigid_body_world _world;
	debug_drawer _ddraw;
	physics_object _phys_wall;
	physics_object _phys_projectile;
};

shared_ptr<btCollisionShape> make_box_shape(btVector3 const & half_length)
{
	return shared_ptr<btCollisionShape>{new btBoxShape{half_length}};
}

scene_window::scene_window()
	: base{parameters{}.name("OpenGL wall - press <space> to shoot")}, _look{_cam, *this}, _move{_cam, *this}
{
	_wall = make_quad_xy();
	_plane = make_quad_xz();
	_prog.from_memory(shaded_shader_source);
	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.position = vec3{0, 1, 5};

	_phys_wall = physics_object{make_box_shape(btVector3{3, 3, 0.1})};
	_world.add(_phys_wall.body());

	_ddraw.toggle_debug_flag(btIDebugDraw::DBG_DrawWireframe);
	_world.debug_drawer(&_ddraw);
	_world.world()->setGravity(btVector3{0,0,0});  // vypnem gravitaciu

	glClearColor(0, 0, 0, 1);
}

scene_window::~scene_window()
{}

btVector3 bullet_cast(vec3 const & v)
{
	return btVector3{v.x, v.y, v.z};
}

void scene_window::fire()
{
	_phys_projectile = physics_object{make_box_shape(btVector3{0.1, 0.1, 0.1}), 1, bullet_cast(_cam.position)};
	_phys_projectile.body()->setLinearVelocity(bullet_cast(_cam.forward()*-2.0f));
	_world.add(_phys_projectile.body());
}

void scene_window::input(float dt)
{
	if (in.key_up('g'))
		_ddraw.toggle_debug_flag(btIDebugDraw::DBG_DrawWireframe);

	if (in.key_up(' '))
		fire();

	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_ddraw.update(_cam);
	_world.simulate(dt);
}

void scene_window::display()
{
	vec3 const light_pos{1,3,1};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	mat4 M = scale(vec3{10,10,10});
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_world = mat3{inverseTranspose(mat4{1})};

	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_prog.uniform_variable("light_dir", normalize(light_pos));
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
//	_wall.render();
//	_plane.render();

	_axis.render(_cam);
	_light.render(_cam, light_pos);
	_world.debug_draw();

	base::display();
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

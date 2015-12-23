// frustum cull test
#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glut_window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "frustum.hpp"
#include "gl/shapes.hpp"

using std::vector;
using std::cos;
using std::sin;
using std::shared_ptr;
using std::string;
using gl::mesh;
using gl::make_quad_xz;
using gl::make_cube;
using gl::camera;
using gl::free_look;
using gl::free_move;

unsigned const num_scene_objects = 24;

string const shader_source{
	"uniform mat4 local_to_screen;\n\
	uniform vec3 color = vec3(0.7, 0.7, 0.7);\n\
	#ifdef _VERTEX_\n\
	layout(location = 0) in vec3 position;\n\
	layout(location = 2) in vec3 normal;\n\
	out vec3 n;  // normal\n\
	void main()	{\n\
		n = normal;\n\
		gl_Position = local_to_screen * vec4(position, 1);\n\
	}\n\
	#endif  // _VERTEX_\n\
	#ifdef _FRAGMENT_\n\
	in vec3 n;  // normal\n\
	out vec4 fcolor;\n\
	const vec3 light_direction = normalize(vec3(1));\n\
	void main()	{\n\
		float light = max(dot(normalize(n), light_direction), 0);\n\
		fcolor = vec4(min(0.3 + light, 1) * color, 1);\n\
	}\n\
	#endif  // _FRAGMENT_"};

struct scene_object
{
	glm::vec3 position;
	bool visible = true;

	scene_object(glm::vec3 const & position) : position(position) {}
};

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	vector<scene_object> _objects;
	mesh _cube;
	mesh _plane;
	shader::program _prog;
	shared_ptr<shader::uniform> _local_to_screen_u;
	shared_ptr<shader::uniform> _color_u;
	camera _cam;
	camera _eagle_eye;
	camera * _curr_cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;
};

scene_window::scene_window() : _look(_cam, *this), _move(_cam, *this)
{
	// [scene objects]
	double r = 10.0;
	double angle = 2*M_PI/num_scene_objects;

	for (int i = 0; i < num_scene_objects; ++i)
		_objects.emplace_back(glm::vec3{r*cos(angle*i), 0, r*sin(angle*i)});

	_cam = camera{glm::vec3{0,1,0}, glm::radians(70.0f), aspect_ratio(), 0.01, 100};
	_eagle_eye = camera{glm::vec3{15,15,15}, glm::radians(70.0f), aspect_ratio(), 0.01, 100};
	_eagle_eye.look_at(glm::vec3{0,0,0});
	_curr_cam = &_cam;

	_cube = make_cube<mesh>();
	_plane = make_quad_xz<mesh>(glm::vec2{0,0}, 30);
	_prog.from_memory(shader_source);
	_local_to_screen_u = _prog.uniform_variable("local_to_screen");
	_color_u = _prog.uniform_variable("color");

	glEnable(GL_DEPTH_TEST);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_prog.use();

	glm::mat4 world_to_screen = _curr_cam->world_to_screen();

	for (scene_object const & o : _objects)
	{
		if (_curr_cam == &_cam && !o.visible)  // apply frustum cull only if regular camera
			continue;

		glm::mat4 local_to_world = glm::translate(glm::mat4{1}, o.position);
		*_local_to_screen_u = world_to_screen * local_to_world;

		if (_curr_cam == &_cam)
			*_color_u = glm::vec3{0.7, 0.7, 0.7};
		else
			*_color_u = o.visible ? glm::vec3{0, 0, 0.7} : glm::vec3{0.7, 0, 0};

		_cube.render();
	}

	*_local_to_screen_u = world_to_screen * glm::translate(glm::mat4{1}, glm::vec3{-15, -1, -15});
	*_color_u = glm::vec3{0x5c/255.0f, 0x36/255.0f, 0x33/255.0f};
	_plane.render();

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);

	// [update objects visibility]
	frustum f{_cam.world_to_screen()};

	for (scene_object & o : _objects)
	{
		if (f.box_in_frustum(o.position - glm::vec3{0.5}, o.position + glm::vec3{0.5}))
			o.visible = true;
		else
			o.visible = false;
	}
}

void scene_window::input(float dt)
{
	if (in.key('1'))
		_curr_cam = &_cam;

	if (in.key('2'))
		_curr_cam = &_eagle_eye;

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

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "gl/phong.hpp"
#include "gl/texture_loader.hpp"

using std::min;
using std::shared_ptr;
using std::vector;
using std::string;
using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::vec4;
using glm::radians;
using glm::translate;
using glm::rotate;
using glm::scale;
using gl::mesh;
using gl::free_camera;
using gl::shape_generator;
using gl::texture_from_file;
using ui::glut_pool_window;


char const * flat_shader_source = R"(
	// #version 330
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(.7, .7, .7);
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	void main() {
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main() {
		fcolor = vec4(color, 1);
	}
	#endif
)";

char const * textured_shader_source = R"(
	// #version 330
	uniform sampler2D s;
	uniform mat4 local_to_screen;
	uniform float intensity = 1.0;
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	out vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = intensity * texture(s, uv);
	}
	#endif
)";

struct planet_characteristics
{
	float semi_major_axis;  // in AU
	float mean_radius;      // in km
	float orbital_period;   // in days
	string name;
};

enum solar_system_object {
	sun, mercury, venus, earth, mars, jupiter, saturn, uranus, neptune, moon, saturn_ring, size
};

planet_characteristics solar_system_characteristics[solar_system_object::size] = {
	{0, 696342, 0, "sun"},
	{.387, 2439.7, 88, "mercury"},
	{.723, 6051.8, 224.7, "venus"},
	{1, 6371, 365.26, "earth"},
	{1.524, 3389.5, 686.97, "mars"},
	{5.2, 69911, 4332.59, "jupiter"},
	{9.55, 58232, 10759.22, "saturn"},
	{19.22, 25362, 30688.5, "uranus"},
	{30.11, 24622, 60182, "neptune"},
	{0.00257, 1737.1, 27.32, "moon"}
};

char const * texture_path[solar_system_object::size] = {
	"assets/textures/sunmap.jpg",  // sun
	"assets/textures/mercurymap.jpg",  // mercury
	"assets/textures/venusmap.jpg",  // venus
	"assets/textures/earthmap1k.jpg",  // earth
	"assets/textures/marsmap1k.jpg",  // mars
	"assets/textures/jupiter2_1k.jpg",  // jupiter
	"assets/textures/saturnmap.jpg",  // saturn
	"assets/textures/uranusmap.jpg",  // uranus
	"assets/textures/neptunemap.jpg",  // neptune
	"assets/textures/moonmap1k.jpg",  // moon
	"assets/textures/saturnringcolor.jpg"  // saturn-ring
};

struct planet
{
	planet_characteristics characteristics;
	vector<planet *> satellites;
	texture2d diffuse_map;  // normal_map, bump_map
	float angle = 0.0f;
	bool has_ring = false;
	texture2d ring_map;

	~planet() {
		for (planet * s : satellites)
			delete s;
	}
};

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;

private:
	planet _planets[9];
	mesh _sphere;
	mesh _circle;
	mesh _ring;
	shader::phong_light _phong;
	shader::program _prog;
	shader::program _flat_prog;
	shader::program _textured_prog;
	axis_object _axis;
	free_camera<scene_window> _cam;
	bool _paused = false;
};

scene_window::scene_window() : _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	shape_generator<mesh> shapes;
	_sphere = shapes.sphere(1.0f, 60, 40);
	_circle = shapes.circle(1.0f, 60);
	_ring = shapes.ring(1.2, 2.0, 60);
	_prog.from_memory(shader::phong_textured_shader_source);
	_phong.init(&_prog, &_cam.get_camera());
	_flat_prog.from_memory(flat_shader_source);
	_textured_prog.from_memory(textured_shader_source);
	_cam.get_camera().position = vec3{-20, 20, 20};
	_cam.get_camera().look_at(vec3{0,0,0});

	for (int i = 0; i < 9; ++i)
	{
		_planets[i].characteristics = solar_system_characteristics[i];
		_planets[i].diffuse_map = texture_from_file(texture_path[i], texture::parameters{}.min(texture_filter::linear_mipmap_linear));
	}

	// moon
	planet * moon = new planet{};
	moon->characteristics = solar_system_characteristics[solar_system_object::moon];
	moon->diffuse_map = texture_from_file(texture_path[solar_system_object::moon], texture::parameters{}.min(texture_filter::linear_mipmap_linear));
	planet & earth = _planets[solar_system_object::earth];
	earth.satellites.push_back(moon);

	// saturn
	planet & saturn = _planets[solar_system_object::saturn];
	saturn.has_ring = true;
	saturn.ring_map = texture_from_file(texture_path[solar_system_object::saturn_ring], texture::parameters{}.min(texture_filter::linear_mipmap_linear));
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// planets
	_phong.light_direction(vec3{0});

	float earth_r = solar_system_characteristics[3].mean_radius;

	// sun
	planet & sun = _planets[0];
	_textured_prog.use();
	sun.diffuse_map.bind(0);
	_textured_prog.uniform_variable("s", 0);
	_textured_prog.uniform_variable("local_to_screen",	_cam.get_camera().world_to_screen() * scale(vec3{2.0f}));
	_textured_prog.uniform_variable("intensity", 0.9f);
	_sphere.render();

	for (int i = 1; i < 9; ++i)
	{
		// planeta
		planet & p = _planets[i];
		planet_characteristics & characteristics = p.characteristics;
		float dist = 10 * characteristics.semi_major_axis;
		float r = min(characteristics.mean_radius/earth_r, 2.0f);
		mat4 S = scale(vec3{r});
		mat4 T = translate(vec3{dist, 0, 0});
		mat4 R = rotate(p.angle, vec3{0,1,0});
		mat4 M = R*T*S;
		vec3 light_dir = -normalize(mat3{R} * vec3{1,0,0});
		shader::program & prog = _phong.shader_program();
		prog.use();
		p.diffuse_map.bind(0);
		prog.uniform_variable("s", 0);
		_phong.light_direction(light_dir);
		_phong.render(_sphere, M);

		// planet sattelites
		for (planet * s : p.satellites)
		{
			mat4 T_local = translate(vec3{/*10 * s->characteristics.semi_major_axis*/2, 0, 0});
			mat4 S_local = scale(vec3{s->characteristics.mean_radius/earth_r});
			mat4 R_local = rotate(s->angle, vec3{0,1,0});
			mat4 local_to_world = M * R_local * T_local * S_local;
			s->diffuse_map.bind(0);
			_phong.render(_sphere, local_to_world);
		}

		// planet ring
		if (p.has_ring)
		{
			mat4 R_local = mat4_cast(angleAxis(radians(15.0f), vec3{1, -1, 1}));
			mat4 local_to_screen = _cam.get_camera().world_to_screen() * M * R_local;
			_textured_prog.use();
			_textured_prog.uniform_variable("local_to_screen", local_to_screen);
			_textured_prog.uniform_variable("s", 0);
			p.ring_map.bind(0);
			_ring.render();
		}

		// obezna draha planety (orbit)
		_flat_prog.use();
		_flat_prog.uniform_variable("local_to_screen", _cam.get_camera().world_to_screen() * scale(vec3{10 * characteristics.semi_major_axis}));
		_flat_prog.uniform_variable("color", vec3{.15});
		_circle.render();
	}

//	_axis.render(_cam.get_camera().world_to_screen());
	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);

	if (_paused)
		return;

	float desired_earth_op = 15.0f;  // in seconds
	float earth_op = solar_system_characteristics[3].orbital_period;

	for (planet & p : _planets)
	{
		float w = (2*M_PI) / (desired_earth_op * p.characteristics.orbital_period/earth_op);  // zem sa otoci za 5s
		p.angle = fmod(p.angle + w*dt, 2*M_PI);
		for (planet * s : p.satellites)
		{
			w = (2*M_PI) / (desired_earth_op * s->characteristics.orbital_period/earth_op);
			s->angle = fmod(s->angle + w*dt, 2*M_PI);
		}
	}
}

void scene_window::input(float dt)
{
	if (in.key_up(' '))
		_paused = _paused ? false : true;
	_cam.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

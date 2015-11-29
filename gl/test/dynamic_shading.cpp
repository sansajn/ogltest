// ukazka sceny s tienovanim s pohyblivym zdrojom svetla
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "texture.hpp"
#include "gl/texture_loader.hpp"
#include "shapes.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::radians;
using glm::inverseTranspose;
using glm::scale;
using glm::translate;
using glm::normalize;
using gl::mesh;
using gl::make_cube;
using gl::make_sphere;
using gl::make_plane_xz;
using gl::camera;
using gl::free_look;
using gl::free_move;

string normal_prog_path = "assets/shaders/geometry_norm.glsl";
string checker_texture_path = "assets/textures/checker8x8.png";

string solid_shader_source = R"(
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
	void main() {
		fcolor = vec4(color, 1);
	}
	#endif
)";

string textured_shader_source = R"(
	// zobrazi otexturovany model (bez osvetlenia)
	uniform mat4 local_to_screen;
	uniform sampler2D s;
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 texcoord;
	out vec2 uv;
	void main()	{
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = texture(s, uv);
	}
	#endif
)";

string shaded_shader_source = R"(
	// zozbrazi model s tienovanim
	uniform mat4 local_to_screen;
	uniform mat3 normal_transform;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,1,1));
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main() {
		n = normal_transform * normal;
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
	using base = ui::glut_pool_impl;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	mesh _cube;
	mesh _light;
	mesh _plane;
	shader::program _shaded_view;
	shader::program _solid_view;
	shader::program _textured_view;
	shader::program _normal_view;
	texture2d _checker;
	camera _cam;
	free_move<scene_window> _move;
	free_look<scene_window> _look;

	// light simulation
	vec3 _light_pos;
	float _angular_pos;
};

scene_window::scene_window() : _move{_cam, *this}, _look{_cam, *this}
{
	_cube = make_cube();
	_light = make_sphere();
	_plane = make_plane_xz(10, 10);
	_shaded_view.from_memory(shaded_shader_source);
	_solid_view.from_memory(solid_shader_source);
	_textured_view.from_memory(textured_shader_source);
	_normal_view.from_file(normal_prog_path);
	_checker = gl::texture_from_file(checker_texture_path, texture::parameters{}.mag(texture_filter::nearest));
	_cam = camera{vec3{3,3,3}, radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.look_at(vec3{0,0,0});

	_angular_pos = 0;
}

void scene_window::display()
{
	vec3 light_dir = normalize(_light_pos);

	// cube
	mat4 M_cube{1};
	mat4 local_to_screen = _cam.view_projection() * M_cube;
	mat3 normal_transform = mat3{inverseTranspose(M_cube)};

	_shaded_view.use();
	_shaded_view.uniform_variable("local_to_screen", local_to_screen);
	_shaded_view.uniform_variable("normal_transform", normal_transform);
	_shaded_view.uniform_variable("light_dir", light_dir);
	_shaded_view.uniform_variable("color", vec3{0.6, 0.4, 0.6});

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_cube.render();

	// plane
	_textured_view.use();
	mat4 M_plane = translate(mat4{1}, vec3{-5, -0.5, 5});
	M_plane = scale(M_plane, vec3{10, 10, 10});
	local_to_screen = _cam.view_projection()*M_plane;
	_textured_view.uniform_variable("local_to_screen", local_to_screen);
	_checker.bind(0);
	_textured_view.uniform_variable("s", 0);
	_plane.render();

	// cube normals
	local_to_screen = _cam.view_projection()*M_cube;
	_normal_view.use();
	_normal_view.uniform_variable("local_to_screen", local_to_screen);
	_normal_view.uniform_variable("normal_length", 0.1f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_cube.render();

	// light
	mat4 M_light = translate(mat4{1}, _light_pos);
	M_light = scale(M_light, vec3{0.1, 0.1, 0.1});

	_solid_view.use();
	_solid_view.uniform_variable("color", vec3{1,1,0});  // yellow
	_solid_view.uniform_variable("local_to_screen", _cam.view_projection()*M_light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_light.render();

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);  // must be first for fps stats

	float const r = 5.0f;
	float const angular_velocity = 2*M_PI/10.0f;

	_angular_pos += angular_velocity * dt;
	if (_angular_pos > 2*M_PI)
		_angular_pos -= 2*M_PI;

	_light_pos = vec3{0,5,0} + vec3{r*cos(_angular_pos), 0, r*sin(_angular_pos)};
}

void scene_window::input(float dt)
{
	_move.input(dt);
	_look.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

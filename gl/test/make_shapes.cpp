// kocka
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
#include "gl/program.hpp"
#include "gl/shapes.hpp"
#include "gl/mesh.hpp"
#include "gl/colors.hpp"
#include "gl/texture_loader.hpp"
#include "gl/glut_free_camera.hpp"

using std::string;
using std::vector;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::normalize;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using gl::mesh;
using gl::shape_generator;
using gl::attribute;
using gl::glut::free_camera;
using gl::texture_from_file;
using ui::glut_pool_window;

char const * saturn_texture_path = "../assets/textures/saturnmap.jpg";
char const * saturn_ring_texture_path = "../assets/textures/saturnringcolor.jpg";

char const * shaded_shader_source = R"(
	// zozbrazi model s tienovanim zalozenom na normale vrchola
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,1,1));

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

char const * textured_shader_source = R"(
	// zozbrazi model s tienovanim zalozenom na normale vrchola
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 light_dir = normalize(vec3(1,1,1));
	uniform sampler2D s;

	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 texcoord;
	layout(location=2) in vec3 normal;
	out vec3 n;
	out vec2 uv;
	void main() {
		uv = texcoord;
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 n;
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = vec4(max(dot(n, light_dir), 0.2) * texture(s, uv).rgb, 1);
	}
	#endif
)";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void input(float dt) override;
	void display() override;

private:
	mesh _cube;
	mesh _box;
	mesh _disk;
	mesh _cylinder;
	mesh _open_cylinder;
	mesh _cone;
	mesh _sphere;
	mesh _circle;
	mesh _ring;
	mesh _saturn, _saturn_ring;
	shader::program _shaded;
	shader::program _textured;
	texture2d _saturn_map, _saturn_ring_map;
	axis_object _axis;
	light_object _light;
	free_camera<scene_window> _cam;
};


scene_window::scene_window()
	: base{parameters{}.name("shape generator sample")}, _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	shape_generator<mesh> shape;
	_cube = shape.cube();
	_box = shape.box(vec3{.5, 1, 0.5});
	_disk = shape.disk(.5);
	_cylinder = shape.cylinder(.5, .5, 30);
	_open_cylinder = shape.open_cylinder(.5, 1, 20);
	_cone = shape.cone(.5, 1);
	_sphere = shape.sphere(.5);
	_circle = shape.circle(.5);
	_ring = shape.ring(.25, .5, 30);
	_saturn = shape.sphere(.5);
	_saturn_ring = shape.ring(.6, 1.1, 50);
	_shaded.from_memory(shaded_shader_source);
	_textured.from_memory(textured_shader_source);
	_saturn_map = texture_from_file(saturn_texture_path);
	_saturn_ring_map = texture_from_file(saturn_ring_texture_path);
	_cam.get_camera().position = vec3{2,2,3.3};
	_cam.get_camera().look_at(vec3{0,0,0});
}

void scene_window::display()
{
	vec3 light_pos = vec3{10, 20, 30};

	_shaded.use();
	// cube
	mat4 VP = _cam.get_camera().view_projection();
	mat4 M = mat4{1};
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::gray);
	_shaded.uniform_variable("light_dir", normalize(light_pos));

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_cube.render();

	// box
	M = translate(vec3{2, 0, -2});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::teal);
	_box.render();

	// disk
	M = translate(vec3{1, 0, 2});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::yellow);
	_disk.render();

	// cylinder
	M = translate(vec3{-1.5, 0, -.4});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::olive);
	_cylinder.render();

	// open cylinder
	M = translate(vec3{-.2, 0, -2});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::maroon);
	_open_cylinder.render();

	// cone
	M = translate(vec3{-2, 0, 1.5});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::purple);
	_cone.render();

	// sphere
	M = translate(vec3{-.7, 0, 1.8});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::blue_shades::cornflower_blue);
	_sphere.render();

	// circle
	M = translate(vec3{2, 0, 1});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::white);
	_circle.render();

	// ring
	M = translate(vec3{-3, 0, .5});
	_shaded.uniform_variable("local_to_screen", VP*M);
	_shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_shaded.uniform_variable("color", rgb::lime);
	_ring.render();

	// saturn
	_textured.use();
	_textured.uniform_variable("light_dir", normalize(light_pos));
	M = translate(vec3{-2, 0, -3});
	_textured.uniform_variable("local_to_screen", VP*M);
	_textured.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_textured.uniform_variable("s", 0);
	_saturn_map.bind(0);
	_saturn.render();
	_saturn_ring_map.bind(0);
	M = M * rotate(radians(20.0f), vec3{0,0,1});
	_textured.uniform_variable("local_to_screen", VP*M);
	_textured.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	_saturn_ring.render();

	_axis.render(_cam.get_camera().view_projection());
	_light.render(_cam.get_camera().view_projection() * translate(light_pos));

	base::display();
}

void scene_window::input(float dt)
{
	_cam.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}


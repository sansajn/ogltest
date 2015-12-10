// kocka
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
#include "gl/program.hpp"
#include "gl/shapes.hpp"
#include "gl/mesh.hpp"
#include "gl/colors.hpp"

using std::string;
using std::vector;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::normalize;
using glm::radians;
using glm::inverseTranspose;
using gl::mesh;
using gl::shape_generator;
using gl::attribute;
using gl::free_camera;
using ui::glut_pool_window;


char const * phong_shader_source = R"(
	// phongov osvetlovaci model (pocitane s priestoru kamery)
	struct directional_light
	{
		vec3 direction;
		vec3 color;
		float intensity;
	};

	struct material_prop
	{
		vec3 ambient;
		float shininess;
		float intensity;  // shininess intensity
	};

	uniform mat4 camera_to_screen;
	uniform mat4 world_to_camera;
	uniform mat4 local_to_world;
	uniform mat3 normal_to_camera;
	uniform directional_light light;
	uniform material_prop material;
	uniform vec3 color;

	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 uv;
	layout(location=2) in vec3 normal;

	out VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
		vec2 uv;
	} vs_out;

	void main()
	{
		mat4 local_to_camera = world_to_camera * local_to_world;
		mat4 local_to_screen = camera_to_screen * local_to_camera;

		vs_out.n = normalize(normal_to_camera * normal);

		vec4 p = local_to_camera * vec4(position,1);
		vs_out.v = -p.xyz;  // view in camera space

		vec3 l = vec3(world_to_camera * vec4(light.direction,0));
		vs_out.l = l;  // light in camera space

		vs_out.uv = uv;
		gl_Position = local_to_screen * vec4(position,1);
	}

	#endif
	#ifdef _FRAGMENT_
	in VS_OUT {
		vec3 v;  // view-direction in camera space (not normalized)
		vec3 l;  // light-direction in camera space (not normalized)
		vec3 n;  // normal-direction in camera space (not normalized)
		vec2 uv;
	} fs_in;

	out vec4 fcolor;

	void main()
	{
		vec3 v = normalize(fs_in.v);
		vec3 l = normalize(fs_in.l);
		vec3 n = normalize(fs_in.n);
		vec3 r = normalize(-reflect(l,n));

		vec3 amb = material.ambient;
		float diff = max(dot(n,l), 0.0) * light.intensity;
		float spec = pow(max(dot(r,v), 0.0), material.shininess) * material.intensity * light.intensity;

		fcolor = vec4(amb*light.color + (diff+spec) * light.color, 1) * vec4(color, 1);
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
	shader::program _prog;
	axis_object _axis;
	light_object _light;
	free_camera<scene_window> _cam;
	shape_generator<mesh> _shape;
};


scene_window::scene_window()
	: base{parameters{}.name("phong light model")}
	, _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_cube = _shape.cube();
	_box = _shape.box(vec3{.5, 1, 0.5});
	_disk = _shape.disk(.5);
	_cylinder = _shape.cylinder(.5, .5, 30);
	_open_cylinder = _shape.open_cylinder(.5, 1, 20);
	_cone = _shape.cone(.5, 1);
	_sphere = _shape.sphere(.5);
	_prog.from_memory(phong_shader_source);
	_cam.get_camera().position = vec3{2,2,3.3};
	_cam.get_camera().look_at(vec3{0,0,0});
}

void scene_window::display()
{
	vec3 light_pos = vec3{10, 20, 30};

	_prog.use();
	// cube
	mat4 M = mat4{1};
	mat4 V = _cam.get_camera().view();
	_prog.uniform_variable("camera_to_screen", _cam.get_camera().camera_to_screen());
	_prog.uniform_variable("world_to_camera", V);
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("light.direction", normalize(light_pos));
	_prog.uniform_variable("light.color", rgb::white);
	_prog.uniform_variable("light.intensity", 1.0f);
	_prog.uniform_variable("material.ambient", vec3{.1});
	_prog.uniform_variable("material.shininess", 4.0f);
	_prog.uniform_variable("material.intensity", .4f);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_cube.render();

	// box
	M = translate(vec3{2, 0, -2});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::teal);
	_box.render();

	// disk
	M = translate(vec3{1, 0, 2});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::yellow);
	_disk.render();

	// cylinder
	M = translate(vec3{-1.5, 0, -.4});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::olive);
	_cylinder.render();

	// open cylinder
	M = translate(vec3{-.2, 0, -2});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::maroon);
	_open_cylinder.render();

	// cone
	M = translate(vec3{-2, 0, 1.5});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::purple);
	_cone.render();

	// sphere
	M = translate(vec3{-.7, 0, 1.8});
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("normal_to_camera", mat3{inverseTranspose(V*M)});
	_prog.uniform_variable("color", rgb::blue_shades::cornflower_blue);
	_sphere.render();

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



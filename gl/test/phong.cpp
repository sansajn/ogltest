// phongov osvetlovaci model
#include <glm/common.hpp>
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/glut_window.hpp"
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "gl/controllers.hpp"
#include "gl/colors.hpp"
#include "gl/scene_object.hpp"
#include "gl/glut_free_camera.hpp"

char const * solid_shader_path = "assets/shaders/solid.glsl";

// phong implementacia (pocitana vo world priestore)
char const * phong_shader_source = R"(
	//#version 330
	uniform mat4 local_to_screen;
	uniform mat4 local_to_world;
	uniform vec3 color;  // object color
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec3 texcoord;
	layout(location = 2) in vec3 normal;
	out VS_OUT {
		vec4 world_position;  // world space position
		vec4 world_normal;  // world space normal
		vec2 texcoord;
	} vs_out;
	void main() {
		vs_out.world_position = local_to_world * vec4(position, 1);
		vs_out.world_normal = local_to_world * vec4(normal, 0);
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	uniform vec4 world_eye_pos;  // eye position in world space
	uniform vec4 world_light_pos;
	uniform vec4 light_color;  // light's diffuse and specular contribution
	uniform vec4 material_emissive;
	uniform vec4 material_diffuse;
	uniform vec4 material_specular;
	uniform float material_shininess;
	uniform vec4 ambient;  // global ambient
	uniform sampler2D s;

	in VS_OUT {
		vec4 world_position;  // world space position
		vec4 world_normal;  // world space normal
		vec2 texcoord;
	} fs_in;

	out vec4 fcolor;

	void main() {
		// emissive term
		vec4 emissive = material_emissive;

		// diffuse term
		vec4 N = normalize(fs_in.world_normal);
		vec4 L = normalize(world_light_pos - fs_in.world_position);
		float NdotL = max(dot(N,L), 0);
		vec4 diffuse = NdotL * light_color * material_diffuse;

		// specular
		vec4 V = normalize(world_eye_pos - fs_in.world_position);
		vec4 H = normalize(L+V);
		vec4 R = reflect(-L,N);
		float RdotV = max(dot(R,V), 0);
		float NdotH = max(dot(N,H), 0);
		vec4 specular = pow(RdotV, material_shininess) * light_color * material_specular;

		fcolor = (emissive + ambient + diffuse + specular) * vec4(color, 1);
	}
	#endif
)";

using glm::mat4;
using glm::vec4;
using glm::vec3;
using glm::radians;

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;

private:
	gl::mesh _planet;
	shader::program _phong;
	gl::glut::free_camera<scene_window> _cam;

	// debug
	axis_object _axis;
	shader::program _solid;
};

scene_window::scene_window() : _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_planet = gl::make_sphere<gl::mesh>(1.0f, 60, 30);
	_phong.from_memory(phong_shader_source);
	_cam.get_camera().position = vec3{0,0,5};
	_solid.from_file(solid_shader_path);
}

void scene_window::update(float dt)
{
	base::update(dt);
}

void scene_window::display()
{
	vec4 const white{1};
	vec4 const black{0};
	vec4 const ambient{.1f, .1f, .1f, 1.0f};

	_phong.use();
	mat4 local_to_world{1};
	mat4 local_to_screen = _cam.get_camera().world_to_screen() * local_to_world;
	_phong.uniform_variable("local_to_screen", local_to_screen);
	_phong.uniform_variable("local_to_world", local_to_world);
	_phong.uniform_variable("color", rgb::blue_shades::light_sky_blue);
	_phong.uniform_variable("world_eye_pos", vec4{_cam.get_camera().position, 1});
	_phong.uniform_variable("world_light_pos", vec4{90,0,0,0}/*local_to_world[3]*/);  // <-- toto je dobra finta local_to_world[3] vrati posunutie transformacie
	_phong.uniform_variable("light_color", white);
	_phong.uniform_variable("material_emissive", black);
	_phong.uniform_variable("material_diffuse", white);
	_phong.uniform_variable("material_specular", white);
	_phong.uniform_variable("material_shininess", 50.0f);
	_phong.uniform_variable("ambient", ambient);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_planet.render();

//	_axis.render(_cam.get_camera().world_to_screen());

	base::display();
}

void scene_window::input(float dt)
{
	base::input(dt);
	_cam.input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

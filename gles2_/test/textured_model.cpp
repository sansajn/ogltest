// TODO: rename to textured_mpdel
// precita model s texturami (gles2 kompatibilnym shaderom), pomocou triedy model
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/free_camera.hpp"
#include "gl/colors.hpp"
#include "gles2/model_loader_gles2.hpp"
#include "gles2/light_gles2.hpp"

char const * model_path = "../../assets/models/bob_lamp/bob_lamp.md5mesh";

using std::string;
using std::shared_ptr;
using std::vector;
using std::runtime_error;
using glm::vec3;
using glm::vec2;
using glm::mat4;
using glm::mat3;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using gl::camera;
using gl::free_camera;
using gles2::model;
using gles2::model_from_file;
using gles2::material_property;
using gles2::phong_light;
using gles2::shader::program;


char const * shader_source = R"(
#ifdef _VERTEX_
	attribute vec3 position;
	attribute vec2 texcoord;
	attribute vec3 normal;
	attribute vec3 tangent;

	uniform mat4 local_to_camera;
	uniform mat4 local_to_screen;
	uniform mat4 world_to_camera;
	uniform mat3 normal_to_camera;

	uniform vec3 light_direction;

	varying vec3 v;  // view direction (tangent-space)
	varying vec3 l;  // light direction
	varying vec2 uv;

	void main() {
		vec3 n = normalize(normal_to_camera * normal);
		vec3 t = normalize(normal_to_camera * tangent);
		vec3 b = cross(n,t);
		mat3 T_tbn = mat3(t,b,n);

		vec4 p = local_to_camera * vec4(position,1);
		v = -p.xyz * T_tbn;

		l = mat3(world_to_camera) * light_direction * T_tbn;

		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
#endif

#ifdef _FRAGMENT_
	precision mediump float;
	uniform sampler2D diff_tex;
	uniform sampler2D norm_tex;
	uniform sampler2D height_tex;

	uniform vec2 parallax_scale_bias;

	uniform vec3 light_color;
	uniform float light_intensity;

	uniform vec3 material_ambient;
	uniform float material_intensity;
	uniform float material_shininess;

	varying vec3 v;  // view direction
	varying vec3 l;  // light direction
	varying vec2 uv;

	void main() {
		vec3 l_norm = normalize(l);
		vec3 n = normalize(texture2D(norm_tex, uv).xyz * 2.0 - 1.0);

		float diff_light = max(dot(n, l_norm), 0.0);

		vec3 v_norm = normalize(v);
		vec3 r = normalize(-reflect(l_norm, v_norm));
		float spec_light = pow(max(dot(r, v_norm), 0.0), material_shininess) * material_intensity;

		vec3 light = material_ambient*light_color + (diff_light + spec_light)*light_color * light_intensity;

		float h = texture2D(height_tex, uv).r * parallax_scale_bias.x + parallax_scale_bias.y;
		vec2 st = uv + h * v_norm.xy;
		vec4 texel = texture2D(diff_tex, st);

		gl_FragColor = vec4(light * texel.xyz, 1);
	}
#endif
)";


class scene_window : public ui::glfw_window
{
public:
	using base = ui::glfw_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	model _mdl;
	free_camera<scene_window> _view;
	program _prog;
	phong_light _light;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	_prog.from_memory(shader_source);

	_mdl = model_from_file(model_path);
	_mdl.append_global(new material_property{vec3{.45}, .8f, 64.0f});

	int position_a = _prog.attribute_location("position");
	int texcoord_a = _prog.attribute_location("texcoord");
	int normal_a = _prog.attribute_location("normal");
	int tangent_a = _prog.attribute_location("tangent");
	_mdl.attribute_location({position_a, texcoord_a, normal_a, tangent_a});

	_view.get_camera().position = vec3{0, 6, 6};
	_view.get_camera().look_at(vec3{0, 3, 0});

	_light = phong_light{normalize(vec3{2,3,5}), rgb::white, .85f};
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{1,0,0});
	mat4 world_to_camera = cam.view();
	mat4 local_to_camera = world_to_camera * M;
	mat4 local_to_screen = cam.projection() * local_to_camera;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera*M)};
	_prog.use();
	_light.apply(_prog);  // apply light setting
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("local_to_camera", local_to_camera);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("parallax_scale_bias", vec2{0.01, -0.01});

	glEnable(GL_DEPTH_TEST);
	_mdl.render(_prog);

	base::display();
}

void scene_window::input(float dt)
{
	_view.input(dt);
	base::input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

// vyzualizuje normal a tangent vektory modelu
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/model_loader.hpp"
#include "gl/program.hpp"
#include "gl/default_shaders.hpp"
#include "gl/controllers.hpp"
#include "gl/scene_object.hpp"
#include "gl/colors.hpp"
#include "gl/shapes.hpp"
#include "gl/mesh.hpp"
#include "gl/animation.hpp"
#include "gl/light.hpp"
#include "gl/label.hpp"
#include "gl/glut_free_camera.hpp"

char const * model_path = "../../assets/models/doom3/imp/imp.md5mesh";
char const * font_path = "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf";


using std::vector;
using std::string;
using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::vec2;
using glm::normalize;
using glm::inverseTranspose;
using glm::radians;
using glm::rotate;
using ui::glut_pool_window;
using gl::model;
using gl::model_from_file;
using gl::model_loader_parameters;
using gl::animated_model;
using gl::animated_model_from_file;
using gl::mesh;
using gl::glut::free_camera;
using gl::camera;
using gl::phong_light;
using gl::skeletal_animation;
using shader::program;
using ui::label;

namespace fs = boost::filesystem;


char const * normal_shader_source = R"(
	// #version 330
	const int MAX_JOINTS = 100;

	uniform mat4 local_to_camera;
	uniform mat4 local_to_screen;
	uniform mat4 world_to_camera;
	uniform mat3 normal_to_camera;

	uniform mat4 skeleton[MAX_JOINTS];  // kostra ako zoznam transformacii

	uniform sampler2D diff_tex;
	uniform sampler2D norm_tex;
	uniform sampler2D height_tex;

	uniform vec2 parallax_scale_bias = vec2(0.04, -0.03);

	uniform vec3 light_direction = normalize(vec3(1,2,3));
	uniform vec3 light_color = vec3(1);
	uniform float light_intensity = 1.0;

	uniform vec3 material_ambient = vec3(.2);
	uniform float material_intensity = 1.0;
	uniform float material_shininess = 64.0;

#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	layout(location = 2) in vec3 normal;
	layout(location = 3) in vec3 tangent;
	layout(location = 4) in ivec4 joints;  // kleleton indices
	layout(location = 5) in vec4 weights;

	out vec3 v;  // view direction (tangent-space)
	out vec3 l;  // light direction
	out vec2 uv;

	void main()
	{
		mat4 T_skin =
			skeleton[joints.x] * weights.x +
			skeleton[joints.y] * weights.y +
			skeleton[joints.z] * weights.z +
			skeleton[joints.w] * weights.w;

		vec4 n_skin = T_skin * vec4(normal_to_camera * normal, 0);
		vec4 t_skin = T_skin * vec4(normal_to_camera * tangent, 0);

		vec3 n = normalize(n_skin.xyz);
		vec3 t = normalize(t_skin.xyz);
		vec3 b = cross(n,t);
		mat3 T_tbn = mat3(t,b,n);

		vec4 p = local_to_camera * T_skin * vec4(position,1);
		v = -p.xyz * T_tbn;

		l = mat3(world_to_camera) * light_direction * T_tbn;

		uv = texcoord;
		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
#endif

#ifdef _FRAGMENT_
	in vec3 v;  // view direction
	in vec3 l;  // light direction
	in vec2 uv;
	out vec4 fcolor;

	void main()
	{
		vec3 l_norm = normalize(l);
		vec3 n = normalize(texture(norm_tex, uv).xyz * 2.0 - 1.0);

		float diff_light = max(dot(n, l_norm), 0.0);

		vec3 v_norm = normalize(v);
		vec3 r = normalize(-reflect(l_norm, v_norm));
		float spec_light = pow(max(dot(r, v_norm), 0.0), material_shininess) * material_intensity;

		vec3 light = material_ambient*light_color + (diff_light + spec_light)*light_color * light_intensity;

		float h = texture(height_tex, uv).r * parallax_scale_bias.x + parallax_scale_bias.y;
		vec2 st = uv + h * v_norm.xy;
		vec4 texel = texture(diff_tex, st);
		fcolor = vec4(light * texel.xyz, 1);
	}
#endif
)";


char const * textured_shader_source = R"(
	// #version 330
	const int MAX_JOINTS = 100;

	uniform mat4 local_to_camera;
	uniform mat4 local_to_screen;
	uniform mat4 world_to_camera;

	uniform mat4 skeleton[MAX_JOINTS];  // kostra ako zoznam transformacii

	uniform sampler2D diff_tex;

	uniform vec3 light_direction = normalize(vec3(1,2,3));
	uniform vec3 light_color = vec3(1);
	uniform float light_intensity = 1.0;

	uniform vec3 material_ambient = vec3(.2);
	uniform float material_intensity = 1.0;
	uniform float material_shininess = 64.0;

#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	layout(location = 2) in vec3 normal;
	layout(location = 3) in vec3 tangent;
	layout(location = 4) in ivec4 joints;  // kleleton indices
	layout(location = 5) in vec4 weights;

	out vec3 v;  // view direction (camera space)
	out vec3 l;  // light direction
	out vec3 n;  // normal
	out vec2 uv;

	void main()
	{
		mat4 T_skin =
			skeleton[joints.x] * weights.x +
			skeleton[joints.y] * weights.y +
			skeleton[joints.z] * weights.z +
			skeleton[joints.w] * weights.w;

		vec4 p = local_to_camera * vec4(position,1);

		v = -p.xyz;
		l = mat3(world_to_camera) * light_direction;
		n = mat3(world_to_camera) * normal;
		uv = texcoord;
		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
#endif

#ifdef _FRAGMENT_
	in vec3 v;  // view direction
	in vec3 l;  // light direction
	in vec3 n;  // normal
	in vec2 uv;
	out vec4 fcolor;

	void main()
	{
		vec3 l_norm = normalize(l);
		vec3 n_norm = normalize(n);

		float diff_light = max(dot(n_norm, l_norm), 0.0);

		vec3 v_norm = normalize(v);
		vec3 r = normalize(-reflect(l_norm, v_norm));
		float spec_light = pow(max(dot(r, v_norm), 0.0), material_shininess) * material_intensity;

		vec3 light = material_ambient*light_color + (diff_light + spec_light)*light_color * light_intensity;

		vec4 texel = texture(diff_tex, uv);
		fcolor = vec4(light * texel.xyz, 1);
	}
#endif
)";


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	animated_model _mdl;
	program _prog;
	program _textured;
	phong_light _light;
	free_camera<scene_window> _view;
	axis_object _axis;
	vector<string> _animations;
	unsigned _cur_animation_idx = 0;
	label<scene_window> _animation_state_lbl;
};

scene_window::scene_window()
	: base{parameters{}.name("local space")}, _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	auto params = model_loader_parameters{};
//	params.file_format = ".tga";
	params.diffuse_texture_postfix = "_d";
//	params.ignore_textures = true;
//	params.ignore_normal_map = true;
//	params.ignore_height_map = true;
	_mdl = animated_model_from_file(model_path, params);

	// animation list
	fs::path model_dir{model_path};
	model_dir.remove_filename();
	for (fs::path const & p : fs::directory_iterator{model_dir})  // TODO: use algorithm
		if (p.extension() == ".md5anim")
			_animations.push_back(p.string());
	assert(!_animations.empty() && "ziadne animacie");

	for (string & a : _animations)
		_mdl.append_animation(skeletal_animation{a});

	_mdl.animation_sequence(vector<unsigned>{_cur_animation_idx});

	_prog.from_memory(normal_shader_source);
	_textured.from_memory(textured_shader_source);
	_light = phong_light{normalize(vec3{2,3,5}), rgb::white, .85f};

	camera & cam = _view.get_camera();
	cam.position = vec3{0, 80, 80};
	cam.look_at(vec3{0, 45, 0});

	_animation_state_lbl.init(0, 0, *this);
	_animation_state_lbl.font(font_path, 12);
	_animation_state_lbl.text(_animations[_cur_animation_idx]);
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{0,1,0}) * rotate(radians(-90.0f), vec3{1,0,0});
	mat4 world_to_camera = cam.view();
	mat4 local_to_camera = world_to_camera * M;
	mat4 local_to_screen = cam.projection() * local_to_camera;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera*M)};

//	auto & prog = _textured;
	auto & prog = _prog;
	prog.use();
	_light.apply(prog);
	prog.uniform_variable("local_to_screen", local_to_screen);
	prog.uniform_variable("local_to_camera", local_to_camera);
	prog.uniform_variable("world_to_camera", world_to_camera);
	prog.uniform_variable("normal_to_camera", normal_to_camera);
	prog.uniform_variable("parallax_scale_bias", vec2{0.01, -0.01});
	prog.uniform_variable("skeleton", _mdl.skeleton());
	_mdl.render(prog);

	_axis.render(cam.world_to_screen());

	_animation_state_lbl.render();

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	_mdl.update(dt);
}

void scene_window::input(float dt)
{
	base::input(dt);

	if (in.key_up(' '))  // space
	{
		_cur_animation_idx = (_cur_animation_idx+1) % _mdl.animation_count();
		_mdl.animation_sequence(vector<unsigned>{_cur_animation_idx});
		_animation_state_lbl.text(_animations[_cur_animation_idx]);
	}

	_view.input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

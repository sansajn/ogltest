// md5 animacia s texturami
#include <string>
#include <vector>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/animation.hpp"
#include "gl/controllers.hpp"
#include "gl/program.hpp"
#include "gl/light.hpp"
#include "gl/scene_object.hpp"

using std::vector;
using std::string;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::radians;
using glm::rotate;
using glm::inverseTranspose;
using ui::glut_pool_window;
using gl::animated_model;
using gl::animated_model_from_file;
using gl::skeletal_animation;
using gl::material_property;
using gl::free_camera;
using gl::camera;
using gl::phong_light;


string const model_path = "../../assets/models/bob_lamp/bob_lamp";


char const * shader_source = R"(
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


class scene_window : public glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;

private:
	shader::program _prog;
	phong_light _light;
	animated_model _mdl;
	free_camera<scene_window> _view;
	axis_object _axis;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_mdl = animated_model_from_file(model_path + ".md5mesh");
	_mdl.append_animation(skeletal_animation{model_path + ".md5anim"});
	_mdl.animation_sequence(vector<unsigned>{0});
	_mdl.append_global(new material_property{vec3{.45}, .8f, 64.0f});

	_prog.from_memory(shader_source);
	_light = phong_light{normalize(vec3{2,3,5}), rgb::white, .85f};
	_view.get_camera().position = vec3{0,3,10};

	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{1,0,0});
	mat4 world_to_camera = cam.view();
	mat4 local_to_camera = world_to_camera * M;
	mat4 local_to_screen = cam.projection() * local_to_camera;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera*M)};

	_prog.use();
	_light.apply(_prog);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("local_to_camera", local_to_camera);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("parallax_scale_bias", vec2{0.01, -0.01});
	_prog.uniform_variable("skeleton", _mdl.skeleton());
	_mdl.render(_prog);

	_axis.render(cam.world_to_screen());

	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	_mdl.update(dt);
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

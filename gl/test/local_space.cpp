// vyzualizuje normal a tangent vektory modelu
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

char const * model_path = "../../assets/models/doom3/imp/imp.md5mesh";

using glm::mat4;
using glm::mat3;
using glm::vec3;
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
using gl::free_camera;
using gl::camera;
using shader::program;


// vyzualizuje normal vektory
char const * normal_shader_path = R"(
	uniform mat4 local_to_screen;
	uniform float normal_length = 0.01;

	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 2) in vec3 normal;

	out Vertex {
		vec3 normal;
	} vertex;

	void main()
	{
		vertex.normal = normal;
		gl_Position = vec4(position,1);
	}
	#endif  // _VERTEX_

	#ifdef _GEOMETRY_
	layout(triangles) in;
	layout(line_strip, max_vertices = 6) out;

	in Vertex {
		vec3 normal;
	} vertex[];

	void main()
	{
		for (int i = 0; i < gl_in.length(); ++i)
		{
			vec3 p = gl_in[i].gl_Position.xyz;
			gl_Position = local_to_screen * vec4(p,1);
			EmitVertex();
			
			vec3 n = vertex[i].normal.xyz;
			gl_Position = local_to_screen * vec4(p + n*normal_length, 1);
			EmitVertex();
			
			EndPrimitive();
		}
	}
	#endif  // _GEOMETRY_

	#ifdef _FRAGMENT_
	out vec4 color;

	void main()
	{
		color = vec4(0,0,1,1);
	}
	#endif  // _FRAGMENT_
)";

// vyzualizuje tangent vektory
char const * tangent_shader_path = R"(
	uniform mat4 local_to_screen;
	uniform float tangent_length = 0.01;

	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 3) in vec3 tangent;

	out Vertex {
		vec3 tangent;
	} vertex;

	void main()
	{
		vertex.tangent = tangent;
		gl_Position = vec4(position,1);
	}
	#endif  // _VERTEX_

	#ifdef _GEOMETRY_
	layout(triangles) in;
	layout(line_strip, max_vertices = 6) out;

	in Vertex {
		vec3 tangent;
	} vertex[];

	void main()
	{
		for (int i = 0; i < gl_in.length(); ++i)
		{
			vec3 p = gl_in[i].gl_Position.xyz;
			gl_Position = local_to_screen * vec4(p,1);
			EmitVertex();

			vec3 t = vertex[i].tangent.xyz;
			gl_Position = local_to_screen * vec4(p + t*tangent_length, 1);
			EmitVertex();

			EndPrimitive();
		}
	}
	#endif  // _GEOMETRY_

	#ifdef _FRAGMENT_
	out vec4 color;

	void main()
	{
		color = vec4(1,0,0,1);
	}
	#endif  // _FRAGMENT_
)";


class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;
	scene_window();
	void display() override;
	void input(float dt) override;

private:
	animated_model _mdl;
	program _shaded;
	program _normal_prog;
	program _tangent_prog;
	free_camera<scene_window> _view;
	axis_object _axis;
};

scene_window::scene_window()
	: base{parameters{}.name("local space")}, _view{radians(70.0f), aspect_ratio(), 0.01f, 1000.0f, *this}
{
	auto params = model_loader_parameters{};
	params.ignore_textures = true;
	_mdl = animated_model_from_file(model_path, params);

	_shaded.from_memory(shader::flat_shaded_shader_source);
	_normal_prog.from_memory(normal_shader_path);
	_tangent_prog.from_memory(tangent_shader_path);

	camera & cam = _view.get_camera();
	cam.position = vec3{0, 100, 60};
	cam.look_at(vec3{0, 55, 0});
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera & cam = _view.get_camera();
	mat4 M = rotate(radians(-90.0f), vec3{0,1,0}) * rotate(radians(-90.0f), vec3{1,0,0}) * mat4{1};
	mat4 world_to_screen = cam.world_to_screen();
	mat4 local_to_screen = world_to_screen * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};
	vec3 color = vec3{rgb::gray};
	vec3 light_dir = normalize(vec3{1,2,3});

	_shaded.use();
	_shaded.uniform_variable("local_to_screen", local_to_screen);
	_shaded.uniform_variable("normal_to_world", normal_to_world);
	_shaded.uniform_variable("color", color);
	_shaded.uniform_variable("light_dir", light_dir);
	_mdl.render(_shaded);

	_normal_prog.use();
	_normal_prog.uniform_variable("local_to_screen", local_to_screen);
	_normal_prog.uniform_variable("normal_length", .1f);
	_mdl.render(_normal_prog);

	_tangent_prog.use();
	_tangent_prog.uniform_variable("local_to_screen", local_to_screen);
	_tangent_prog.uniform_variable("tangent_length", .1f);
	_mdl.render(_tangent_prog);

	_axis.render(world_to_screen);

	base::display();
}

void scene_window::input(float dt)
{
	base::input(dt);
	_view.input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

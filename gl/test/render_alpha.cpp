// render transparentnej textury
#include <string>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "window.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "shapes.hpp"
#include "gl/texture_loader.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::scale;
using gl::mesh;
using gl::make_quad_xy;
using ui::glut_pool_window;

string texture_shader_source = R"(
	uniform mat4 local_to_screen;
	uniform sampler2D diff_tex;
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 texcoord;
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
		fcolor = texture(diff_tex, uv);
	}
	#endif
)";

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;

private:
	mesh _quad;
	texture2d _crosshair_tex;
	shader::program _prog;
};

scene_window::scene_window()
{
	_quad = make_quad_xy<mesh>();
	_crosshair_tex = gl::texture_from_file("assets/textures/crosshair_blaster.tga",
		texture::parameters().min(texture_filter::nearest).mag(texture_filter::nearest));
	_prog.from_memory(texture_shader_source);
	glClearColor(0,0,0,1);
}

void scene_window::display()
{
	// TODO: comu zodpovedaju rozmery obrazka v px ?
	mat4 local_to_screen = scale(mat4{1}, 0.04f * vec3{1, aspect_ratio(), 1});
	_prog.use();
	_crosshair_tex.bind(0);
	_prog.uniform_variable("diff_tex", 0);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	_quad.render();
	base::display();
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

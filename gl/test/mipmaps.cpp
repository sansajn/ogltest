// generovanie mipmap textury
#include <algorithm>
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/shapes.hpp"
#include "gl/program.hpp"
#include "gl/window.hpp"
#include "gl/controllers.hpp"

char const * shader_program_source = R"(
	//#version 420
	uniform mat4 local_to_screen;
	layout(binding = 0) uniform sampler2D s;
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	out vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif  // _VERTEX_
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = vec4(texture(s, uv).xxx, 1);
	}
	#endif  // _FRAGMENT_
)";

using std::max;
using glm::vec3;
using glm::radians;
using gl::mesh;
using gl::free_camera;
using gl::make_plane_xz;

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	GLuint _tbo;  // texture
	mesh _plane;
	shader::program _prog;
	free_camera<scene_window> _cam;
};

GLuint make_mipmap_texture()
{
	GLuint tbo;
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);

	int mipmap_levels = 5;
	glTexStorage2D(GL_TEXTURE_2D, mipmap_levels, GL_R8, 64, 64);  // allocate storage for 5 levels of base texture 64x64, 32x32, 16x16, 8x8, 4x4

	int w = 64;
	for (int i = 0; i < mipmap_levels; ++i)
	{
		int intensity = (i+1)*255/mipmap_levels;
		uint8_t * pixels = new uint8_t[w*w];
		memset(pixels, intensity, w*w);
		glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, w, w, GL_RED, GL_UNSIGNED_BYTE, pixels);
		w = max(1, w/2);
		delete [] pixels;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
	return tbo;
}

scene_window::scene_window() : _cam{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_tbo = make_mipmap_texture();
	_plane = make_plane_xz(10, 10, 10.0);
	_prog.from_memory(shader_program_source, 420);
	_cam.get_camera().position = vec3{0, 3, 0};
	_cam.get_camera().look_at(vec3{10,0,-10});
	glEnable(GL_DEPTH_TEST);
}

void scene_window::display()
{
	glm::mat4 M = glm::scale(glm::mat4(1), glm::vec3(5,5,5));
	glm::mat4 local_to_screen = _cam.get_camera().world_to_screen() * M;

	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _tbo);
	_prog.uniform_variable("s", 0);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	_plane.render();

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
	std::cout << "done!" << std::endl;
	return 0;
}


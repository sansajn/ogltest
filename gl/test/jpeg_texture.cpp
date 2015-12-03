// citanie jpeg suboru
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <csetjmp>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <jpeglib.h>
#include "gl/program.hpp"
#include "gl/texture.hpp"
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "pix/pix_jpeg.hpp"

char const * image_path = "assets/textures/lena.jpg";

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::swap;
using gl::mesh;

char const * shader_source = R"(
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	out vec2 st;
	void main() {
		st = position.xy/2.0 + 0.5;
		gl_Position = vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	uniform sampler2D s;
	in vec2 st;
	out vec4 fcolor;
	void main() {
		fcolor = texture(s, st);
	}
	#endif
)";


pixel_format match_pixel_format(uint8_t channels)
{
	switch (channels)
	{
		case 3: return pixel_format::rgb;
		case 4: return pixel_format::rgba;
		case 1: return pixel_format::red;
		default:
			throw std::logic_error{"unable to match a pixel-format"};
	}
}

pixel_type match_pixel_type(uint8_t depth)
{
	switch (depth)
	{
		case 1: return pixel_type::ub8;
		case 2: return pixel_type::us16;
		case 4: return pixel_type::ui32;
		default:
			throw std::logic_error{"unable to match a pixel-type"};
	}
}

texture2d jpeg_texture_from_file(string const & fname)
{
	pix::jpeg_decoder d;
	d.decode(fname);
	pix::flip(d.result.height, d.result.rowbytes, d.result.pixels);
	return texture2d{d.result.width, d.result.height, sized_internal_format::rgba8,
		match_pixel_format(d.result.channels), match_pixel_type(d.result.depth), d.result.pixels};
}


void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	shader::program prog;
	prog.from_memory(shader_source);

	texture2d tex = jpeg_texture_from_file(image_path);
	mesh texframe = gl::make_quad_xy<mesh>();

	// render
	prog.use();
	tex.bind(0);
	prog.uniform_variable("s", 0);
	texframe.render();

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL texture demo");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

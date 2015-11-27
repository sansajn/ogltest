// na obrazovku vykresli png texturu
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <png.h>
#include "gl/program.hpp"
#include "gl/texture.hpp"
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "pix_png.hpp"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::swap;
using gl::mesh;

//char const * file_name = "assets/textures/checker_rgba_4x4.png";
char const * file_name = "assets/textures/lena.png";

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


texture2d png_texture_from_file_with_decoder(string const & fname)
{
	pix::png_decoder d;
	d.decode_as_rgba8(fname);
	pix::flip(d.result.height, d.result.rowbytes, d.result.pixels);
	return texture2d{d.result.width, d.result.height, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, d.result.pixels};
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

	texture2d tex = png_texture_from_file_with_decoder(file_name);
	mesh texframe = gl::make_quad_xy();

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

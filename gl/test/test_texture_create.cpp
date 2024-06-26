// vytvorenie texturi s pamete
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "program.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include <Magick++.h>
#include "gl/shapes.hpp"

using std::vector;
using std::pair;
using std::make_pair;
using gl::mesh;
using gl::make_quad_xy;

std::string const shader_source{
	"#ifdef _VERTEX_\n\
	layout(location=0) in vec3 position;\n\
	out vec2 st;\n\
	void main() {\n\
		st = position.xy/2.0 + 0.5;\n\
		gl_Position = vec4(position, 1);\n\
	}\n\
	#endif\n\
	#ifdef _FRAGMENT_\n\
	uniform sampler2D s;\n\
	in vec2 st;\n\
	out vec4 fcolor;\n\
	void main() {\n\
		fcolor = texture(s, st).rrra;\n\
	}\n\
	#endif"};

void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	shader::program prog;
	prog.from_memory(shader_source);

	uint8_t data[16] = {0x00, 0xff, 0x00, 0xff,
							  0xff, 0x00, 0xff, 0x00,
							  0x00, 0xff, 0x00, 0xff,
							  0xff, 0x00, 0xff, 0x00};

	texture2d tex(4, 4, sized_internal_format::r8, pixel_format::red,	pixel_type::ub8,
		data, texture::parameters().mag(texture_filter::nearest));

	mesh texframe = make_quad_xy<mesh>();

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


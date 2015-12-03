// na obrazovku vykresli texturu pomocou triedy texture
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "program.hpp"
#include "texture.hpp"
#include "gl/texture_loader.hpp"
#include "mesh.hpp"
#include "gl/shapes.hpp"

using std::vector;
using std::pair;
using std::make_pair;
using gl::mesh;

std::string const & texture_path = "assets/textures/lena.png";
//std::string const & texture_path = "assets/textures/bricks.png";

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

void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	shader::program prog;
	prog.from_memory(shader_source);

	texture2d tex = gl::texture_from_file(texture_path);
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

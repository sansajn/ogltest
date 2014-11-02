// nakresí trojuholník iba za použitia gl-funkcií (bez transformácie)
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "render/program.hpp"
#include "render/texture2d_array.hpp"
#include "render/cpubuffer.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * fs_src = "uniform sampler2DArray sampler;\n\
							  uniform vec2 scale;\n\
							  uniform int layer;\n\
							  out vec4 fcolor;\n\
							  void main() {\n\
								  fcolor = texture(sampler, vec3(gl_FragCoord.xy * scale, layer)).rrrr;\n\
							  }\n";

struct framebuffer
{
	static void program_use(program & p) {p.set();}
};

void init(int argc, char * argv[]);


int main(int argc, char * argv[])
{
	init(argc, argv);

	ptr<module> simple_modules(new module(330, nullptr, fs_src));
	program prog(simple_modules);

	GLfloat vertices[6*2] = {
		// t1
		-1, -1,
		 1, -1,
		 1,  1,
		 // t2
		 -1, -1,
		  1,  1,
		 -1,  1
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, 6*2*sizeof(GLfloat), (GLvoid *)vertices, GL_STATIC_DRAW);

	GLuint position_attr_id = 0;
	glVertexAttribPointer(position_attr_id, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attr_id);

	uint8_t checker_pattern[32] = {
		// checker 1
		  0, 255,   0, 255,
		255,   0, 255,   0,
		  0, 255,   0, 255,
		255,   0, 255,   0,
		// checker 2
		255, 255, 255, 255,
		255,   0,   0, 255,
		255,   0,   0, 255,
		255, 255, 255, 255
	};

	ptr<texture2D_array> tex(new texture2D_array(4, 4, 2, texture_internal_format::R8,
		texture_format::RED, pixel_type::UNSIGNED_BYTE, texture::parameters().mag(texture_filter::NEAREST),
		buffer::parameters(), cpubuffer(checker_pattern)));

	ptr<uniform2f> scale_u = prog.get_uniform<uniform2f>("scale");
	ptr<uniform1i> layer_u = prog.get_uniform<uniform1i>("layer");
	ptr<uniform_sampler> sampler_u = prog.get_uniform<uniform_sampler>("sampler");

	// rendering ...
	framebuffer::program_use(prog);

	scale_u->set(glm::vec2(1.0f/800.0f, 1.0f/600.0f));
	layer_u->set(2);
	sampler_u->set(tex);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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
	glutCreateWindow("OpenGL texture test");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

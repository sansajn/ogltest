// citanie png textury
#include <string>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/program.hpp"
#include "gl/mesh.hpp"
#include "pix/pix_png.hpp"
#include "gles2/texture_gles2.hpp"

std::string texture_path = "../../assets/textures/lena.png";

char const * shader_source = R"(
	//	#version 330
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	out vec2 st;
	void main() {
		st = position.xy/2.0 + 0.5;
		gl_Position = vec4(position, 1.0f);
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
gl::mesh create_mesh();
gles2::texture2d create_texture(std::string const & fname);


int main(int argc, char * argv[])
{
	init(argc, argv);

	shader::program prog;
	prog.from_memory(shader_source, 330);
	gl::mesh quad = create_mesh();

	gles2::texture2d tex = create_texture(texture_path);

	// rendering ...
	prog.use();
	tex.bind(0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	quad.render();
	assert(glGetError() == GL_NO_ERROR && "opengl error");

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

gles2::texture2d create_texture(std::string const & fname)
{
	pix::png_decoder d;
	d.decode(fname);
	pix::flip(d.result.height, d.result.rowbytes, d.result.pixels);
	return gles2::texture2d{d.result.width, d.result.height, gles2::pixel_format::rgba, gles2::pixel_type::ub8, d.result.pixels};
}

gl::mesh create_mesh()
{
	GLfloat vertices[] = {
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0};

	GLuint indices[] = {
		0, 1, 2,  2, 3, 0
	};

	auto result = gl::mesh{vertices, sizeof(vertices), indices, 6};
	result.attach_attributes({gl::attribute{0, 3, GL_FLOAT, 3*sizeof(GLfloat)}});
	return result;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL texture loading sample");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}
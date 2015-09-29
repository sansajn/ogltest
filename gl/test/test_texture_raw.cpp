#include <string>
#include <cassert>
#include <GL/glew.h>
#include "window.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "texture.hpp"

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
		fcolor = vec4(texture(s, st).rrr, 1);\n\
	}\n\
	#endif"};

using base_window = ui::window<ui::glut_event_impl>;

class main_window : public base_window
{
public:
	main_window();
	void display() override;

private:
	GLuint _tid;
	shader::program _prog;
};

main_window::main_window()
{
	glGenTextures(1, &_tid);
	_prog.from_memory(shader_source);
}

void main_window::display()
{
	glClearColor(.0f, .0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// \note texturu o velkosti 2x2 (gray) nezobrazi spravne (texturu 4x4 uz zobrazi)

	uint8_t pixels[4*3] = {      0,0,0, 255,255,255,
								  255,255,255,       0,0,0};

	_prog.use();

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, _tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // treba nastavit, inak glTexImage2D nezobrazi nic
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	assert(glGetError() == GL_NO_ERROR && "opengl error");

	_prog.uniform_variable("s", 0);

	static mesh quad = make_quad_xy();
	quad.draw();

	base_window::display();
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

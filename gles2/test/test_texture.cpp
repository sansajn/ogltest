// gles2 texture sample
#include <iostream>
#include <cassert>
#include <GLFW/glfw3.h>
#include "gles2/mesh_gles2.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/texture_loader_gles2.hpp"

#define LOG_TAG "loadingpng"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;

using std::string;

string texture_path = "../../assets/textures/lena.png";

char const * textured_shader_source = R"(
	uniform sampler2D s;
	#ifdef _VERTEX_
	attribute vec3 position;
	varying vec2 st;
	void main() {
		st = position.xy/2.0 + 0.5;
		gl_Position = vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	varying vec2 st;
	void main() {
		gl_FragColor = texture2D(s, st);
	}
	#endif
)";

using std::cout;
using gles2::mesh;
using gles2::shader::program;
using gles2::texture2d;

mesh * quad = nullptr;
program * prog = nullptr;
texture2d * tex = nullptr;

mesh * make_quad()
{
	GLfloat vertices[] = {
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0
	};

	GLuint indices[] = {0, 1, 2,  2, 3, 0};

	GLint position_loc = glGetAttribLocation(prog->id(), "position");
	assert(position_loc != -1 && "unknown attribute");

	mesh * result = new mesh{vertices, sizeof(vertices), indices, 6};
	result->append_attribute(gles2::attribute(position_loc, 3, GL_FLOAT, 3*sizeof(GLfloat)));
	return result;
}

void setup_scene()
{
	prog = new program;
	try {
		prog->from_memory(textured_shader_source);
	}
	catch (std::exception & e) {
		cout << e.what() << std::endl;
		return;
	}

	quad = make_quad();
	tex = new texture2d{gles2::texture_from_file(texture_path)};
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	prog->use();
	tex->bind(0);
	prog->uniform_variable("s", 0);
	quad->render();
}

void cleanup_scene()
{
	delete tex;
	delete prog;
	delete quad;
}

int main(int argc, char * argv[])
{
	GLFWwindow* window;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
	glfwMakeContextCurrent(window);

	cout << "GL_VERSION  : " << glGetString(GL_VERSION) << "\n"
		<< "GL_RENDERER : " << glGetString(GL_RENDERER) << std::endl;

	setup_scene();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, WIDTH, HEIGHT);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		render();
		glfwSwapBuffers(window);
	}
	cleanup_scene();
	glfwTerminate();
	return EXIT_SUCCESS;
}

// gles2::shader::program sample
#include <stdio.h>
#include <stdlib.h>
#include "gles2/program_gles2.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;

char const * shader_module_source = R"(
	// #version 100
	#ifdef _VERTEX_
	attribute vec3 position;
	void main() {
		gl_Position = vec4(position, 1.0);
	}
	#endif  // _VERTEX_
	#ifdef _FRAGMENT_
	void main() {
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	#endif  // _FRAGMENT_
)";

static const GLfloat vertices[] = {
	0.0f,  0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
};

int main(int argc, char * argv[])
{
	GLuint vbo;
	GLFWwindow* window;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
	glfwMakeContextCurrent(window);

	printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
	printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

	// tests
	gles2::shader::program prog_;
	prog_.from_memory(shader_module_source, 100);
	GLint pos = prog_.attribute_location("position");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, WIDTH, HEIGHT);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		prog_.use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
	}
	glDeleteBuffers(1, &vbo);
	glfwTerminate();
	return EXIT_SUCCESS;
}

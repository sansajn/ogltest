// gl::camera sample
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/matrix_inverse.hpp>
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"
#include "gl/camera.hpp"
#include "gl/colors.hpp"
#include "gl/shapes.hpp"

static const GLuint WIDTH = 800;
static const GLuint HEIGHT = 600;

using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::radians;
using glm::inverseTranspose;

int main(int argc, char * argv[])
{
	GLFWwindow* window;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
	glfwMakeContextCurrent(window);

	printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
	printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

	gl::camera cam{radians(70.0f), WIDTH/(float)HEIGHT, 0.01f, 1000.0f};
	cam.position = vec3{2,2,5};
	cam.look_at(vec3{0,0,0});

	gles2::shader::program shaded;
	shaded.from_memory(gles2::flat_shaded_shader_source, 100);
	GLint pos = shaded.attribute_location("position");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnableVertexAttribArray(pos);

	gl::shape_generator<gles2::mesh> shape;
	gles2::mesh cube = shape.cube();

	vec3 light_pos = vec3{10, 20, 30};

	shaded.use();

	mat4 VP = cam.view_projection();
	mat4 M = mat4{1};
	shaded.uniform_variable("local_to_screen", VP*M);
	shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
	shaded.uniform_variable("color", rgb::gray);
	shaded.uniform_variable("light_dir", normalize(light_pos));

	GLint position_a = glGetAttribLocation(shaded.id(), "position");
	GLint normal_a = glGetAttribLocation(shaded.id(), "normal");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		cube.attribute_location({position_a, normal_a});
		cube.render();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return EXIT_SUCCESS;
}

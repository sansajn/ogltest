// gl::shape_generator sample
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
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
using glm::translate;
using gles2::mesh;

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
	mesh cube = shape.cube();
	mesh box = shape.box(vec3{.5, 1, 0.5});
	mesh disk = shape.disk(.5);
	mesh cylinder = shape.cylinder(.5, .5, 30);
	mesh open_cylinder = shape.open_cylinder(.5, 1, 20);
	mesh cone = shape.cone(.5, 1);
	mesh sphere = shape.sphere(.5);
	mesh circle = shape.circle(.5);
	mesh ring = shape.ring(.25, .5, 30);

	vec3 light_pos = vec3{10, 20, 30};

	shaded.use();

	GLint position_a = glGetAttribLocation(shaded.id(), "position");
	GLint normal_a = glGetAttribLocation(shaded.id(), "normal");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// cube
		mat4 VP = cam.view_projection();
		mat4 M = mat4{1};
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::gray);
		shaded.uniform_variable("light_dir", normalize(light_pos));

		cube.attribute_location({position_a, normal_a});
		cube.render();

		// box
		M = translate(vec3{2, 0, -2});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::teal);
		box.attribute_location({position_a, normal_a});
		box.render();

		// disk
		M = translate(vec3{1, 0, 2});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::yellow);
		disk.attribute_location({position_a, normal_a});
		disk.render();

		// cylinder
		M = translate(vec3{-1.5, 0, -.4});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::olive);
		cylinder.attribute_location({position_a, normal_a});
		cylinder.render();

		// open cylinder
		M = translate(vec3{-.2, 0, -2});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::maroon);
		open_cylinder.attribute_location({position_a, normal_a});
		open_cylinder.render();

		// cone
		M = translate(vec3{-2, 0, 1.5});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::purple);
		cone.attribute_location({position_a, normal_a});
		cone.render();

		// sphere
		M = translate(vec3{-.7, 0, 1.8});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::blue_shades::cornflower_blue);
		sphere.attribute_location({position_a, normal_a});
		sphere.render();

		// circle
		M = translate(vec3{2, 0, 1});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::white);
		circle.attribute_location({position_a, normal_a});
		circle.render();

		// ring
		M = translate(vec3{-3, 0, .5});
		shaded.uniform_variable("local_to_screen", VP*M);
		shaded.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
		shaded.uniform_variable("color", rgb::lime);
		ring.attribute_location({position_a, normal_a});
		ring.render();

		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return EXIT_SUCCESS;
}

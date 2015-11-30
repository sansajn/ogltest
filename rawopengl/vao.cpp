// použitie vertex-array-object (VAO)
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * vs_src = R"(
	#version 330
	layout(location = 0) in vec3 position;
	layout(location = 2) in vec3 normal;
	uniform mat4 T[3];  // M, V, P transformation
	uniform mat3 normal_to_view;
	out vec3 n;
	void main() {
		n = normal_to_view * normal;
		gl_Position = T[2] * T[1] * T[0] * vec4(position, 1);
	}
)";

char const * fs_src = R"(
	#version 330
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	in vec3 n;
	out vec4 fcolor;
	vec3 light_dir = normalize(vec3(1,1,1));
	void main() {
		fcolor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
	}
)";

void init(int argc, char * argv[]);
void install_shader_program();
void make_mesh();
void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

GLuint vao, vbo, ibo;  // for mesh object
GLuint prog, vs, fs;

void display()
{
	// rendering ...
	glUseProgram(prog);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(vao);

	glm::mat4 P = glm::perspective(glm::radians(60.0f), 800.0f/600.0f, 0.1f, 100.0f);
	glm::vec3 campos(5.0f, 5.0f, 5.0f);
	glm::vec3 origin(0.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(campos, origin, up);
	glm::mat4 M(1.0f);
	glm::mat4 T[3] = {M, V, P};
	glm::mat3 normal_to_view = glm::mat3{glm::inverseTranspose(V*M)};

	glm::vec3 color{0.6, 0.2, 0.3};

	GLint T_loc = glGetUniformLocation(prog, "T[0]");
	assert(T_loc != -1 && "unknown uniform");
	glUniformMatrix4fv(T_loc, 3, GL_FALSE, glm::value_ptr(*T));  // upload 3 matrices
	assert(glGetError() == GL_NO_ERROR);

	GLint normal_transform_loc = glGetUniformLocation(prog, "normal_to_view");
	assert(normal_transform_loc != -1 && "unknown uniform");
	glUniformMatrix3fv(normal_transform_loc, 1, GL_FALSE, glm::value_ptr(normal_to_view));
	assert(glGetError() == GL_NO_ERROR);

	GLint color_loc = glGetUniformLocation(prog, "color");
	assert(color_loc != -1 && "unknown uniform");
	glUniform3fv(color_loc, 1, glm::value_ptr(color));
	assert(glGetError() == GL_NO_ERROR);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	assert(glGetError() == GL_NO_ERROR);

	glBindVertexArray(0);

	glutSwapBuffers();
}

void make_mesh()
{
	GLfloat positions[24*3] = {
		// front
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f,
		// right
		 1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, 1.0f,
		// back
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		// left
		-1.0f, -1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		// bottom
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, 1.0f,
		// top
		-1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f
	};

	GLfloat normals[24*3] = {
		// front
		 0,0,1,
		 0,0,1,
		 0,0,1,
		 0,0,1,
		// right
		 1,0,0,
		 1,0,0,
		 1,0,0,
		 1,0,0,
		// back
		 0,0,-1,
		 0,0,-1,
		 0,0,-1,
		 0,0,-1,
		// left
		-1,0,0,
		-1,0,0,
		-1,0,0,
		-1,0,0,
		// bottom
		 0,-1,0,
		 0,-1,0,
		 0,-1,0,
		 0,-1,0,
		// top
		 1,0,0,
		 1,0,0,
		 1,0,0,
		 1,0,0
	};

	GLuint indices[36] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	// create vao describing mesh attributes
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// vertex buffer object <- position:3, normal:3
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(GLfloat)*2, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24*3*sizeof(GLfloat), (GLvoid *)positions);
	glBufferSubData(GL_ARRAY_BUFFER, 24*3*sizeof(GLfloat), 24*3*sizeof(GLfloat), (GLvoid *)normals);

	GLuint position_loc = 0;  // predefined in vertex shader
	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_loc);

	GLuint normal_loc = 2;  // predefined in vertex shader
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(24*3*sizeof(GLfloat)));
	glEnableVertexAttribArray(normal_loc);

	// create index bufffer object
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);  // bind default vao
}

int main(int argc, char * argv[])
{
	init(argc, argv);
	install_shader_program();
	make_mesh();  // stored in vao, vbo and ibo

	glutMainLoop();

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteProgram(prog);
	glDeleteVertexArrays(1, &vao);

	return 0;
}

void install_shader_program()
{
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, nullptr);
	glCompileShader(vs);

	// check for compile errors
	GLint compiled;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(vs, "vertex-shader");

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, nullptr);
	glCompileShader(fs);

	// check for compile errors ...
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(fs, "fragment-shader");

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	// check for link errors ...
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(prog, "vertex-shader;fragment-shader");
}

void dump_compile_log(GLuint shader, std::string const & name)
{
	GLint len;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	std::string log;
	log.resize(len);
	glGetShaderInfoLog(shader, len, nullptr, (GLchar *)log.data());
	std::cout << "compile output ('" << name << "'):\n" << log << std::endl;
}

void dump_link_log(GLuint program, std::string const & name)
{
	GLint len;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
	std::string log;
	log.resize(len);
	glGetProgramInfoLog(program, len, nullptr, (GLchar *)log.data());
	std::cout << "link output ('" << name << "'):\n" << log << std::endl;
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL VAO test");
	glutDisplayFunc(display);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

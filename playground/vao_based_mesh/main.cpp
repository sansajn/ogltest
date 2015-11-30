#include <vector>
#include <cassert>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/shapes.hpp"

namespace experimental {

using std::vector;
using std::initializer_list;
using std::move;
using std::swap;

class mesh  //!< vao based mesh implementation
{
public:
	mesh();
	mesh(void const * vbuf, unsigned vbuf_size, unsigned const * ibuf, unsigned ibuf_size, gl::buffer_usage usage = gl::buffer_usage::static_draw);
	mesh(mesh && other);
	~mesh();
	void attach_attributes(initializer_list<gl::attribute> attribs);
	void render();
	void operator=(mesh && other);

	mesh(mesh const &) = delete;
	void operator=(mesh const &) = delete;

private:
	unsigned _vao;
	gl::gpu_buffer _vbuf, _ibuf;  //!< vertex and index buffers
	unsigned _nindices;
	int _draw_mode;  //!< GL_POINTS, GL_LINES, GL_TRIANGLES, ... \sa glDrawElements()
};

mesh::mesh() : _vao{0}
{}

mesh::mesh(void const * vbuf, unsigned vbuf_size, unsigned const * ibuf, unsigned ibuf_size, gl::buffer_usage usage)
	: _vao{0}, _vbuf{vbuf, vbuf_size, usage}, _ibuf(ibuf, ibuf_size*sizeof(unsigned), usage), _nindices{ibuf_size}, _draw_mode{GL_TRIANGLES}
{}

mesh::mesh(mesh && other)
	: _vao{other._vao}
	, _vbuf{move(other._vbuf)}
	, _ibuf{move(other._ibuf)}
	, _nindices{other._nindices}
	, _draw_mode{other._draw_mode}
{
	other._vao = 0;
}

void mesh::operator=(mesh && other)
{
	swap(_vao, other._vao);
	swap(_nindices, other._nindices);
	_vbuf = move(other._vbuf);
	_ibuf = move(other._ibuf);
	swap(_draw_mode, other._draw_mode);
}

mesh::~mesh()
{
	glDeleteVertexArrays(1, &_vao);
}

void mesh::attach_attributes(initializer_list<gl::attribute> attribs)
{
	// setup vertex array object for later use
	assert(!_vao && "attributes already attached");

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	_vbuf.bind(gl::buffer_target::array);
	_ibuf.bind(gl::buffer_target::element_array);

	for (gl::attribute const & a : attribs)
	{
		if (a.int_type)
			glVertexAttribIPointer(a.index, a.size, a.type, a.stride, (GLvoid *)(intptr_t)a.start_idx);
		else
			glVertexAttribPointer(a.index, a.size, a.type, a.normalized, a.stride, (GLvoid *)(intptr_t)a.start_idx);

		glEnableVertexAttribArray(a.index);
	}

	glBindVertexArray(0);  // unbind vao
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::render()
{
	assert(_vao && "attributes not attached");
	glBindVertexArray(_vao);
	glDrawElements(_draw_mode, _nindices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);  // unbind vao
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh make_mesh()
{
	GLfloat vertices[24*(3+3)] = {  // position:3, normal:3
		// front
		-1, -1,  1,  0,0,1,
		 1, -1,  1,  0,0,1,
		 1,  1,  1,  0,0,1,
		-1,  1,  1,  0,0,1,
		// right
		 1, -1,  1,  1,0,0,
		 1, -1, -1,  1,0,0,
		 1,  1, -1,  1,0,0,
		 1,  1,  1,  1,0,0,
		// back
		-1, -1, -1,  0,0,-1,
		-1,  1, -1,  0,0,-1,
		 1,  1, -1,  0,0,-1,
		 1, -1, -1,  0,0,-1,
		// left
		-1, -1,  1,  -1,0,0,
		-1,  1,  1,  -1,0,0,
		-1,  1, -1,  -1,0,0,
		-1, -1, -1,  -1,0,0,
		// bottom
		-1, -1,  1,  0,-1,0,
		-1, -1, -1,  0,-1,0,
		 1, -1, -1,  0,-1,0,
		 1, -1,  1,  0,-1,0,
		// top
		-1,  1,  1,  1,0,0,
		 1,  1,  1,  1,0,0,
		 1,  1, -1,  1,0,0,
		-1,  1, -1,  1,0,0
	};

	GLuint indices[36] = {
		0,1,2,  0,2,3,
		4,5,6,  4,6,7,
		8,9,10,  8,10,11,
		12,13,14,  12,14,15,
		16,17,18,  16,18,19,
		20,21,22,  20,22,23
	};

	unsigned stride = (3+3)*sizeof(GLfloat);

	mesh result{vertices, sizeof(vertices), indices, 36};
	result.attach_attributes({
		gl::attribute{0, 3, GL_FLOAT, stride, 0},
		gl::attribute{2, 3, GL_FLOAT, stride, 3*sizeof(GLfloat)}});

	return result;
}

}  // experimental

char const * shader_source = R"(
	//#version 330
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_view;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main() {
		n = normal_to_view * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	vec3 light_dir = normalize(vec3(1,1,1));
	void main() {
		fcolor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
	}
	#endif
)";

void init(int argc, char * argv[]);

int main(int argc, char * argv[])
{
	init(argc, argv);

	shader::program prog;
	prog.from_memory(shader_source);

	experimental::mesh m = experimental::make_mesh();

	prog.use();

	glm::mat4 P = glm::perspective(glm::radians(60.0f), 800.0f/600.0f, 0.1f, 100.0f);
	glm::vec3 campos(5.0f, 5.0f, 5.0f);
	glm::vec3 origin(0.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(campos, origin, up);
	glm::mat4 M(1.0f);
	glm::mat4 local_to_screen = P * V * M;
	glm::mat3 normal_to_view = glm::mat3{glm::inverseTranspose(V*M)};
	glm::vec3 color{0.6, 0.2, 0.3};

	prog.uniform_variable("local_to_screen", local_to_screen);
	prog.uniform_variable("normal_to_view", normal_to_view);
	prog.uniform_variable("color", color);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	m.render();

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
	glutCreateWindow("OpenGL vao based mesh ...");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

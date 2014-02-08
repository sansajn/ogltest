#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <iostream>

#include <boost/format.hpp>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using std::ifstream;
using std::stringstream;
using std::string;
using std::unique_ptr;
using std::cout;
using std::cerr;
using std::endl;


class vbo_cube
{
public:
	vbo_cube();
	void render();

private:
	GLuint _vao;
};

struct glsl_program_exception : public std::runtime_error
{
	glsl_program_exception(std::string const & msg)
		: std::runtime_error(msg)
	{}
};

class glsl_program
{
public:
	glsl_program();
	~glsl_program();
	void compile(char const * filename, GLenum type);
	void link();
	void use() const;
	bool linked() const {return _linked;}
	void uniform(char const * name, glm::mat4 const & m);

private:
	std::string read_shader(char const * filename);
	void create_program_lazy();
	GLint uniform_location(char const * name);

	GLuint _program;
	std::map<std::string, GLint> _uniforms;
	bool _linked;
};

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

void init_glew()
{
	glewExperimental = GL_TRUE;  // inak vo verzii 1.8 padne na glGenVertexArrays()
	GLenum state = glewInit();
	if (state != GLEW_OK)
		cerr << "Error: " << glewGetErrorString(state) << endl;

	if (GLEW_VERSION_4_0)
		cout << "OpenGL 4.0 supported\n";

	glGetError();  // swallow error 1282
	GL_CHECK_ERRORS;
}

void dump_hw_info()
{
	cout << "\tUsing GLEW "<< glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main(int argc, char * argv[])
{
	int w = 800, h = 600;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(w, h);
	glutCreateWindow("vbo cube test");

	init_glew();

	dump_hw_info();

	cout << "gl initialized\n";

	glsl_program prog;
	prog.compile("simple.vs", GL_VERTEX_SHADER);
	prog.compile("simple.fs", GL_FRAGMENT_SHADER);
	prog.link();
	prog.use();

	GL_CHECK_ERRORS;

	cout << "program compiled, linked and used\n";

	glm::mat4 project = glm::perspective(60.0f, float(w)/h, 0.3f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.25f, 1.25f), 
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model = glm::mat4(1.0f);

	glm::mat4 mvp = project*view*model;

	cout << "matrices created\n";

	prog.uniform("mvp", mvp);

	vbo_cube cube;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	cube.render();

	GL_CHECK_ERRORS;

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}


vbo_cube::vbo_cube()
	: _vao(0)
{
	float side = 1.0f;
	float side2 = side/2.0f;

	float v[24*3] = {
		// Front
		-side2, -side2, side2,
		 side2, -side2, side2,
		 side2,  side2, side2,
		-side2,  side2, side2,
		// Right
		 side2, -side2, side2,
		 side2, -side2, -side2,
		 side2,  side2, -side2,
		 side2,  side2, side2,
		// Back
		-side2, -side2, -side2,
		-side2,  side2, -side2,
		 side2,  side2, -side2,
		 side2, -side2, -side2,
		// Left
		-side2, -side2, side2,
		-side2,  side2, side2,
		-side2,  side2, -side2,
		-side2, -side2, -side2,
		// Bottom
		-side2, -side2, side2,
		-side2, -side2, -side2,
		 side2, -side2, -side2,
		 side2, -side2, side2,
		// Top
		-side2,  side2, side2,
		 side2,  side2, side2,
		 side2,  side2, -side2,
		-side2,  side2, -side2
	};

	float n[24*3] = {
		// Front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		// Back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		// Left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		// Bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		// Top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint el[] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	glGenVertexArrays(1, &_vao);  // vytvor 1 vertex-array
	glBindVertexArray(_vao);

	unsigned int bufs[3];
	glGenBuffers(3, bufs);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(float), n, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), el, 
		GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void vbo_cube::render()
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


string shader_info_log(GLuint shader);


glsl_program::glsl_program()
	: _program(0), _linked(false)
{}

glsl_program::~glsl_program()
{
	if (_program == 0)
		return;

	GLint nshaders = 0;
	glGetProgramiv(_program, GL_ATTACHED_SHADERS, &nshaders);

	unique_ptr<GLuint[]> shaders(new GLuint[nshaders]);
	glGetAttachedShaders(_program, nshaders, NULL, shaders.get());

	for (int i = 0; i < nshaders; ++i)
		glDeleteShader(shaders[i]);

	glDeleteProgram(_program);
}

void glsl_program::compile(char const * filename, GLenum type)
{
	create_program_lazy();

	string source = read_shader(filename);

	GLuint shader = glCreateShader(type);
	char const * src = source.c_str();
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	// error handling
	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		string log = shader_info_log(shader);
		throw glsl_program_exception(boost::str(boost::format(
			"can't compile '%1%' shader, reason: %2%") % filename % log));
	}

	glAttachShader(_program, shader);	
}

void glsl_program::link()
{
	if (_linked)
		return;

	if (_program < 1)
		throw glsl_program_exception("program has not been compiled");

	glLinkProgram(_program);

	// error handling
	GLint status;
	glGetProgramiv(_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
		throw glsl_program_exception("program link failed");

	_linked = true;
}

void glsl_program::use() const
{
	if (!_linked)
		throw glsl_program_exception("program has not been linked");

	glUseProgram(_program);
}

void glsl_program::uniform(char const * name, glm::mat4 const & m)
{
	GLint loc = uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

GLint glsl_program::uniform_location(char const * name)
{
	auto it = _uniforms.find(name);
	if (it == _uniforms.end())
		_uniforms[name] = glGetUniformLocation(_program, name);
	return _uniforms[name];
}

void glsl_program::create_program_lazy()
{
	if (_program < 1)
		_program = glCreateProgram();

	if (_program == 0)
		throw glsl_program_exception("unable to create shader program");
}

std::string glsl_program::read_shader(char const * filename)
{
	ifstream in(filename);
	stringstream ss;
	ss << in.rdbuf();
	in.close();
	return ss.str();
}

string shader_info_log(GLuint shader)
{
	GLint length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		unique_ptr<char[]> buf(new char[length]);
		GLint written = 0;
		glGetShaderInfoLog(shader, length, &written, buf.get());
		return string(buf.get());
	}
	else
		return "";
}




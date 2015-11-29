// vytvorenie 2D textúry pomocou glTexStorage2D()
#include <string>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Magick++.h>

//std::string texture_path = "assets/textures/checkboard.png";
std::string texture_path = "assets/textures/lena.jpg";

char const * vs_src = R"(
	#version 330
	layout(location=0) in vec3 position;
	out vec2 st;
	void main() {
		st = position.xy/2.0 + 0.5;
		gl_Position = vec4(position, 1.0f);
	}
)";

char const * fs_src = R"(
	#version 330
	uniform sampler2D s;
	in vec2 st;
	out vec4 fcolor;
	void main() {
	  fcolor = texture(s, st);
	}
)";

void init(int argc, char * argv[]);
void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);
void create_shader_program();
void create_mesh();
GLuint create_texture(std::string const & fname);

GLuint vs, fs, prog;
GLuint vao, vbo, ibo;
unsigned index_count;


int main(int argc, char * argv[])
{
	init(argc, argv);

	create_shader_program();
	create_mesh();
	GLuint tbo = create_texture(texture_path);

	// rendering ...
	glUseProgram(prog);

	GLuint position_attr_id = 0;
	glVertexAttribPointer(position_attr_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attr_id);

	GLint s_loc = glGetUniformLocation(prog, "s");
	assert(s_loc != -1 && "unknown uniform");
	glUniform1i(s_loc, 0);
	glActiveTexture(GL_TEXTURE0);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

	assert(glGetError() == GL_NO_ERROR && "opengl error");

	glutSwapBuffers();

	glutMainLoop();

	glDeleteTextures(1, &tbo);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);
	glDeleteVertexArrays(1, &vao);

	return 0;
}


GLuint create_texture(std::string const & fname)
{
	Magick::Image im{fname};
	im.flip();
	Magick::Blob imblob;
	im.write(&imblob, "RGBA");  // load image as rgba array

	GLuint tbo;
	glGenTextures(1, &tbo);
	glBindTexture(GL_TEXTURE_2D, tbo);
	// glPixelStorei() : adresa kazdeho riadku obrazka je zarovnana 4mi (RGBA foramt)
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, im.columns(), im.rows());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, im.columns(), im.rows(), GL_RGBA, GL_UNSIGNED_BYTE, imblob.data());

	return tbo;
}

void create_mesh()
{
	GLfloat vertices[] = {
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0,
		-1,  1, 0};

	GLuint indices[] = {
		0, 1, 2,  2, 3, 0
	};

	index_count = sizeof(indices)/sizeof(GLuint);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), indices, GL_STATIC_DRAW);
}

void create_shader_program()
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

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL texture loading sample");

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
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

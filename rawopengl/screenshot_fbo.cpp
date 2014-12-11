// screenshot s custom framebuffer-u
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * vs_src = "#version 330\n\
							  layout(location=0) in vec3 position;\n\
							  layout(location=1) in vec4 color;\n\
							  out VS_OUT {\n\
								  vec4 color;\n\
							  } vs_out;\n\
							  void main() {\n\
								  vs_out.color = color;\n\
								  gl_Position = vec4(position, 1.0f);\n\
							  }\n";

char const * fs_src = "#version 330\n\
							  in VS_OUT {\n\
								  vec4 color;\n\
							  } fs_in;\n\
							  out vec4 fcolor;\n\
							  void main() {\n\
								  fcolor = fs_in.color;\n\
							  }\n";

void take_screenshot(std::string const & fname, GLenum mode = GL_BACK);
void init(int argc, char * argv[]);
void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

// framebuffer zapíše ako tga súbor
void take_screenshot(std::string const & fname, GLenum mode)
{
	int viewport[4];  // x,y,w,h
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	assert(w%4 == 0 && "w musi bit nasobkom 4");

#pragma pack(push, 1)
	struct
	{
		unsigned char identsize;  // sile of following ID field
		unsigned char cmaptype;  // color map type (0 = none)
		unsigned char imagetype;  // (2 = rgb)
		short smapstart;  // first entry in palete
		short cmapsize;  // number of entries in palete
		unsigned char cmapbpp;  // number of bits per palette entry
		short xorigin;
		short yorigin;
		short width;
		short height;
		unsigned char bpp;  // bits per pixel
		unsigned char descriptor;
	} tga_header;
#pragma pack(pop)

	int row_size = w*3;
	int data_size = row_size*h;
	std::unique_ptr<unsigned char[]> data(new unsigned char[data_size]);

	glReadBuffer(mode);

	glReadPixels(
		0, 0,  // origin
		w, h, GL_BGR, GL_UNSIGNED_BYTE, data.get());

	memset(&tga_header, 0, sizeof(tga_header));
	tga_header.imagetype = 2;
	tga_header.width = (short)w;
	tga_header.height = (short)h;
	tga_header.bpp = 24;

	std::ofstream fout(fname.c_str(), std::ios::out|std::ios::binary);
	if (!fout.is_open())
		return;

	fout.write((char *)&tga_header, sizeof(tga_header));
	fout.write((char *)data.get(), data_size);
	fout.close();
}


int main(int argc, char * argv[])
{
	init(argc, argv);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, nullptr);
	glCompileShader(vs);

	// check for compile errors
	GLint compiled;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(vs, "vertex-shader");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, nullptr);
	glCompileShader(fs);

	// check for compile errors ...
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
		dump_compile_log(fs, "fragment-shader");

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);	

	// check for link errors ...
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(prog, "vertex-shader;fragment-shader");


	GLfloat vertices[] = {
		-.5f, -.5f, .0f,
		.5f, -.5f, .0f,
		.0f, .5f, .0f};

	GLfloat colors[] = {
		1.0f, .0f, .0f, 1.0f,
		.0f, 1.0f, .0f, 1.0f,
		.0f, .0f, 1.0f, 1.0f};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint tribuf;
	glGenBuffers(1, &tribuf);
	glBindBuffer(GL_ARRAY_BUFFER, tribuf);
	glBufferData(GL_ARRAY_BUFFER, 7*3*sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3*3*sizeof(GLfloat), (GLvoid *)vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3*3*sizeof(GLfloat), 3*4*sizeof(GLfloat), (GLvoid *)colors);

	GLuint position_attr_id = 0, color_attr_id = 1;
	glVertexAttribPointer(position_attr_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attr_id);

	glVertexAttribPointer(color_attr_id, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(3*3*sizeof(GLfloat)));
	glEnableVertexAttribArray(color_attr_id);

	// custom framebuffer
	int w_fb = 1920;
	int h_fb = 1080;

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint color_texture;  // color buffer texture
	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w_fb, h_fb);

	// disable mipmaps for texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint depth_texture;  // depth buffer texture
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, w_fb, h_fb);

	// attach the color and depth textures to the FBO
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);  // draw into the framebuffer's color

	// rendering ...
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, w_fb, h_fb);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(prog);
	glDrawArrays(GL_TRIANGLES, 0, 3);
//	glutSwapBuffers();

	take_screenshot("screenshot_fbo.tga", GL_COLOR_ATTACHMENT0);

	glutMainLoop();

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);

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
	glutCreateWindow("OpenGL triangle");

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


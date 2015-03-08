// phongov osvetlovací model (na kocke nieje zrejmá specular zložka)
#include <string>
#include <cassert>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char const * vs_src = "#version 330\n\
							  layout(location=0) in vec3 position;\n\
							  layout(location=1) in vec3 normal;\n\
							  uniform mat4 MV;\n\
							  uniform mat4 P;\n\
							  uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);\n\
							  out VS_OUT {\n\
								  vec3 n;\n\
								  vec3 l;\n\
								  vec3 v;\n\
							  } vs_out;\n\
							  void main() {\n\
								  vec4 p = MV * vec4(position, 1.0);\n\
								  vs_out.n = mat3(MV) * normal;\n\
								  vs_out.l = light_pos - p.xyz;\n\
								  vs_out.v = -p.xyz;\n\
								  gl_Position = P*p;\n\
							  }\n";

char const * fs_src = "#version 330\n\
							  in VS_OUT {\n\
								  vec3 n;\n\
								  vec3 l;\n\
								  vec3 v;\n\
							  } fs_in;\n\
							  uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);\n\
							  uniform vec3 specular_albedo = vec3(0.7);\n\
							  uniform float specular_power = 128.0;\n\
							  uniform vec3 ambient = vec3(0.1);\n\
							  out vec4 fcolor;\n\
							  void main() {\n\
								  vec3 n = normalize(fs_in.n);\n\
								  vec3 l = normalize(fs_in.l);\n\
								  vec3 v = normalize(fs_in.v);\n\
								  vec3 r = reflect(-l, n);\n\
								  vec3 diffuse = max(dot(n,l), 0.0)*diffuse_albedo;\n\
								  vec3 specular = pow(max(dot(r,v), 0.0), specular_power)*specular_albedo;\n\
								  fcolor = vec4(ambient + diffuse + specular, 1.0);\n\
							  }\n";

void init(int argc, char * argv[]);
void display();
void reshape(int w, int h);
void dump_compile_log(GLuint shader, std::string const & name);
void dump_link_log(GLuint program, std::string const & name);

GLuint prog;
GLint p_loc, mv_loc, lpos_loc, diff_loc, spec_loc, specpow_loc, ambient_loc;


glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 origin(0.0f, 0.0f, 0.0f);
glm::vec3 campos(5.0f, 5.0f, 5.0f);

void display()
{
	glm::mat4 V = glm::lookAt(campos, origin, up);
	glm::mat4 M(1.0f);

	glm::mat4 P = glm::perspective(60.0f, 800.0f/600.0f, 0.1f, 100.0f);
	glm::mat4 MV = V*M;

	glUniformMatrix4fv(p_loc, 1, GL_FALSE, glm::value_ptr(P));
	glUniformMatrix4fv(mv_loc, 1, GL_FALSE, glm::value_ptr(MV));
		
	// light and mateial properties	
	glm::vec3 light_pos(100.0f, 100.0f, 100.0f);
	glm::vec3 diffuse_albedo(0.5f, 0.2f, 0.7f);
	glm::vec3 specular_albedo(0.7f, 0.7f, 0.7f);
	float specular_power = 128.0f;
	glm::vec3 ambient(0.1f, 0.1f, 0.1f);

	glUniform3fv(lpos_loc, 1, glm::value_ptr(light_pos));
	glUniform3fv(diff_loc, 1, glm::value_ptr(diffuse_albedo));
	glUniform3fv(spec_loc, 1, glm::value_ptr(specular_albedo));
	glUniform1f(specpow_loc, specular_power);
	glUniform3fv(ambient_loc, 1, glm::value_ptr(ambient));

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glutSwapBuffers();
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

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	// check for link errors ...
	GLint linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
		dump_link_log(prog, "vertex-shader;fragment-shader");

	GLfloat vertices[24*3] = {
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
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		// right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		// back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		// left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		// bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		// top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint indices[36] = {
		0,1,2,0,2,3,
		4,5,6,4,6,7,
		8,9,10,8,10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buf[2];  // vertices and normals
	glGenBuffers(2, buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, 2*24*3*sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24*3*sizeof(GLfloat), (GLvoid *)vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 24*3*sizeof(GLfloat), 24*3*sizeof(GLfloat), (GLvoid *)normals);

	GLuint position_attr_id = 0, normal_attr_id = 1;
	glVertexAttribPointer(position_attr_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attr_id);

	glVertexAttribPointer(normal_attr_id, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(24*3*sizeof(GLfloat)));
	glEnableVertexAttribArray(normal_attr_id);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

	glEnable(GL_DEPTH_TEST);

	glUseProgram(prog);

	p_loc = glGetUniformLocation(prog, "P");
	mv_loc = glGetUniformLocation(prog, "MV");
	lpos_loc = glGetUniformLocation(prog, "light_pos");
	diff_loc = glGetUniformLocation(prog, "diffuse_albedo");
	spec_loc = glGetUniformLocation(prog, "specular_albedo");
	specpow_loc = glGetUniformLocation(prog, "specular_power");
	ambient_loc = glGetUniformLocation(prog, "ambient");

	assert(p_loc != -1 && "unknown uniform");
	assert(mv_loc != -1 && "unknown uniform");
	assert(lpos_loc != -1 && "unknown uniform");
	assert(diff_loc != -1 && "unknown uniform");
	assert(spec_loc != -1 && "unknown uniform");
	assert(specpow_loc != -1 && "unknown uniform");
	assert(ambient_loc != -1 && "unknown uniform");

	glutMainLoop();

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);

	return 0;
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

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void init(int argc, char * argv[])
{
	// glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitContextVersion(4, 0);
	glutInitContextFlags(GLUT_CORE_PROFILE|GLUT_DEBUG);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL phong lighting");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	// glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	assert(err == GLEW_OK && "glew init failed");
	glGetError();  // eat error
}

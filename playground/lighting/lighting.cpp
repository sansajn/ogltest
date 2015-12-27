/*! \file lighting.cpp
V okne zobrazi nasvieteny a rotujuci model. */
#include <vector>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AntTweakBar.h>
#include "core/ptr.hpp"
#include "render/framebuffer.h"
#include "render/meshbuffers.h"
#include "render/program.hpp"
#include "resource/assimp_loader.h"
#include "ui/glut_window.h"

int const WIDTH = 800;
int const HEIGHT = 600;
char const MODEL_FILE[] = "model/sphere.ply";

class main_window : public glut_window  // TODO: sdl okono nie je resizable
{
public:
	typedef glut_window base;

	main_window();
	void display() override;
	void reshape(int w, int h) override;

	void mouse_motion(int x, int y) override;
	void mouse_click(button b, state s, int x, int y) override;
	void key_typed(unsigned char c, int x, int y) override;

private:
	void load_program(shader_program * prog);
	ptr<mesh_buffers> load_model_mesh() const;
	void init_tweakbar();

	ptr<mesh_buffers> _mesh;
	glm::mat4 _P;  // perspective transformation
	glm::mat4 _V;  // view transformation

	bool _fragment_phong;

	shader_program _prog_vertex;
	shader_program _prog_fragment;
	shader_program * _prog_used;

	uniform_variable _mv_matrix_u;
	uniform_variable _proj_matrix_u;
	uniform_variable _light_pos_u;
	uniform_variable _diffese_albedo_u;
	uniform_variable _specular_albedo_u;
	uniform_variable _specular_power_u;
	uniform_variable _ambient_u;

	// light and material
	glm::vec3 _diffuse_albedo;
	glm::vec3 _specular_albedo;
	float _specular_power;  // shininess
	glm::vec3 _ambient;
	bool _diffuse_enable;
	bool _specular_enable;
	bool _ambient_enable;

	TwBar * _twbar;
	framebuffer & _fb;
	GLuint _vao;
};  // main_window

glm::vec3 zeros{0.0f, 0.0f, 0.0f};
glm::vec3 origin(zeros);
glm::vec3 up{0.0f, 1.0f, 0.0f};
glm::vec3 campos{2.0f, 4.0f, 3.0f};

void main_window::display()
{
	if (_fragment_phong && (_prog_used == &_prog_vertex))  // changed to vertex
	{
		_prog_used = &_prog_fragment;
		load_program(_prog_used);
	}
	else if (!_fragment_phong && (_prog_used == &_prog_fragment))  // changed to fragment
	{
		_prog_used = &_prog_vertex;
		load_program(_prog_used);
	}

	_fb.clear(true, true);

	glBindVertexArray(_vao);
	_prog_used->use();

	glm::mat4 M = glm::mat4(1.0f);
	glm::mat4 MV = _V*M;

	_mv_matrix_u = MV;
	_proj_matrix_u = _P;
	_diffese_albedo_u = _diffuse_enable ? _diffuse_albedo : zeros;
	_specular_albedo_u = _specular_enable ? _specular_albedo : zeros;
	_specular_power_u = _specular_power;
	_ambient_u = _ambient_enable ? _ambient : zeros;

	_mesh->draw();

	glBindVertexArray(0);
	TwDraw();
	base::display();
}

main_window::main_window()
	: base(parameters().name("lighting").size(WIDTH, HEIGHT).debug(true))
	, _fb(framebuffer::default_fb())
{
	glGenVertexArrays(1, &_vao);  // TODO: bez vertex-arrays to nejede
	glBindVertexArray(_vao);

	_mesh = load_model_mesh();
	_P = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.1f, 100.0f);
	_V = glm::lookAt(campos, origin, up);
	_fragment_phong = true;
	_diffuse_albedo = glm::vec3(0.5f, 0.2f, 0.7f);
	_specular_albedo = glm::vec3(0.7f, 0.7f, 0.7f);
	_specular_power = 96.0f;
	_ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	_diffuse_enable = true;
	_specular_enable = true;
	_ambient_enable = true;

	init_tweakbar();

	_prog_vertex << "shader/phong_vertex.vs"  << "shader/phong_vertex.fs";
	_prog_vertex.link();

	_prog_fragment << "shader/phong_fragment.vs" << "shader/phong_fragment.fs";
	_prog_fragment.link();

	_prog_used = _fragment_phong ? &_prog_fragment : &_prog_vertex;

	load_program(_prog_used);

	_fb.depth_test(true);  // TODO: ak by sa po vytvorení okna zavolal reshape
}

void main_window::load_program(shader_program * prog)
{
	prog->use();  // TODO: uniform sa da ziskat aj bez toho aby bol program pouzivany
	_mv_matrix_u.link("mv_matrix", *prog);
	_proj_matrix_u.link("proj_matrix", *prog);
	_light_pos_u.link("light_pos", *prog);
	_diffese_albedo_u.link("diffuse_albedo", *prog);
	_specular_albedo_u.link("specular_albedo", *prog);
	_specular_power_u.link("specular_power", *prog);
	_ambient_u.link("ambient", *prog);
}

void main_window::init_tweakbar()
{
	// tweak-bar
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(WIDTH, HEIGHT);
	_twbar = TwNewBar("Phong Shading Model");
	TwAddVarRW(_twbar, "Mode", TW_TYPE_BOOLCPP, &_fragment_phong, "true=fragment false=vertex");
	TwAddVarRW(_twbar, "DiffuseEnable", TW_TYPE_BOOLCPP, &_diffuse_enable, "group=Diffuse label=Enable");
	TwAddVarRW(_twbar, "DiffuseAlbedo", TW_TYPE_COLOR3F, glm::value_ptr(_diffuse_albedo), "group=Diffuse label=Albedo");
	TwAddVarRW(_twbar, "SpecularEnable", TW_TYPE_BOOLCPP, &_specular_enable, "group=Specular label=Enable");
	TwAddVarRW(_twbar, "SpecularAlbedo", TW_TYPE_COLOR3F, glm::value_ptr(_specular_albedo), "group=Specular label=Albedo");
	TwAddVarRW(_twbar, "SpecularPower", TW_TYPE_FLOAT, &_specular_power, "group=Specular label=Power min=1 max=128 step=1");
	TwAddVarRW(_twbar, "AmbientEnable", TW_TYPE_BOOLCPP, &_ambient_enable, "group=Amgient label=Enable");
	TwAddVarRW(_twbar, "AmbientColor", TW_TYPE_COLOR3F, glm::value_ptr(_ambient), "group=Amgient label=Color");
}

void main_window::reshape(int w, int h)
{
	// TODO: defaultna implementacia nic nerobi, co je skoda (aspon nastavit glViewPort() by mohla
}

ptr<mesh_buffers> main_window::load_model_mesh() const
{
	assimp_loader loader;  // TODO: nazov assimp zamaskuj, co tak common_mesh_loader ?
	return loader.load(MODEL_FILE);
}

void main_window::mouse_motion(int x, int y)
{
	TwMouseMotion(x, y);
	base::mouse_motion(x, y);
}

void main_window::mouse_click(button b, state s, int x, int y)
{
	TwMouseMotion(x, y);

	TwMouseAction action = (s == state::down) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
	switch (b)
	{
		case button::left:
			TwMouseButton(action, TW_MOUSE_LEFT);
			break;

		case button::middle:
			TwMouseButton(action, TW_MOUSE_MIDDLE);
			break;

		case button::right:
			TwMouseButton(action, TW_MOUSE_RIGHT);
			break;

		default:
			// TODO: support wheel events
			break;
	}

	base::mouse_click(b, s, x, y);
}

void main_window::key_typed(unsigned char c, int x, int y)
{
	TwKeyPressed(c, 0);  // TODO: modifiers not handled
	base::key_typed(c, x, y);
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

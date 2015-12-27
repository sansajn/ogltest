#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include "core/ptr.h"
#include "render/meshbuffers.h"
#include "render/program.h"
#include "resource/assimp_loader.h"
#include "ui/glut_window.h"

using std::cout;

int const WIDTH = 800;
int const HEIGHT = 600;

class app_window : public glut_window
{
public:
	typedef glut_window base;

	app_window();
	~app_window();

	void display() override;
	void mouse_motion(int x, int y) override;
	void mouse_passive_motion(int x, int y) override;
	void mouse_wheel(wheel b, int x, int y) override;
	void mouse_click(button b, state s, int x, int y) override;
	void key_typed(unsigned char c, int x, int y) override;
	void key_released(unsigned char c, int x, int y) override;
	void special_key(key k, int x, int y) override;
	void special_key_released(key k, int x, int y) override;

private:
	TwBar * _twbar;
	int _ncubes, _w, _h;
	float _bgcolor[3];
	GLuint _vao;
	ptr<mesh_buffers> _model;
	shader_program _prog;
	glm::mat4 _P;  // perspective transformation
};

glm::vec3 origin{0.0f, 0.0f, 0.0f};
glm::vec3 up{0.0f, 1.0f, 0.0f};
glm::vec3 campos{0.0f, 0.0f, 5.0f};

app_window::app_window()
	: base(parameters().size(WIDTH, HEIGHT).name("AntTwekBar test"))
{
	_ncubes = 10;
	_w = WIDTH;
	_h = HEIGHT;
	_bgcolor[0] = 0.2f; _bgcolor[1] = 0.4f; _bgcolor[2] = 0.6f;
	_P = glm::perspective(60.0f, float(WIDTH)/HEIGHT, 0.1f, 100.0f);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	TwInit(TW_OPENGL, NULL);
	TwWindowSize(WIDTH, HEIGHT);

	_twbar = TwNewBar("test-twakbar");
	TwAddVarRO(_twbar, "Width", TW_TYPE_INT32, &_w, "help='window width'");
	TwAddVarRO(_twbar, "Height", TW_TYPE_INT32, &_h, "help='window height'");
	TwAddVarRW(_twbar, "Cubes", TW_TYPE_INT8, &_ncubes, "min=5 max=20 help='number of cubes to display'");
	TwAddVarRW(_twbar, "Background", TW_TYPE_COLOR3F, _bgcolor, "");

	_prog << "shaders/nolighting.vs" << "shaders/nolighting.fs";
	_prog.link();
	_prog.use();

	assimp_loader loader;
	_model = loader.load("models/monkey.ply");
}

app_window::~app_window()
{
	TwDeleteBar(_twbar);
	TwTerminate();
}

void app_window::display()
{
	glClearColor(_bgcolor[0], _bgcolor[1], _bgcolor[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glm::mat4 M(1.0);
	glm::mat4 V = glm::lookAt(campos, origin, up);
	glm::mat4 MVP = _P*V*M;

	glBindVertexArray(_vao);
	_prog.use();

	uniform_variable mvp_u("local_to_screen", _prog);
	mvp_u = MVP;

	_model->draw();  // #1

	glBindVertexArray(0);

	TwDraw();        // #2

	// riadky 1 a 2 sa spolu nemusia

	base::display();
}

void app_window::mouse_motion(int x, int y)
{
	TwMouseMotion(x, y);
	base::mouse_motion(x, y);
}

void app_window::mouse_passive_motion(int x, int y)
{
	base::mouse_passive_motion(x, y);
}

void app_window::mouse_wheel(wheel b, int x, int y)
{
	base::mouse_wheel(b, x, y);
}

void app_window::mouse_click(button b, state s, int x, int y)
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

void app_window::key_typed(unsigned char c, int x, int y)
{
	TwKeyPressed(c, 0);  // TODO: modifiers not handled
	base::key_typed(c, x, y);
}

void app_window::key_released(unsigned char c, int x, int y)
{
	base::key_released(c, x, y);
}

void app_window::special_key(key k, int x, int y)
{
	base::special_key(k, x, y);
}

void app_window::special_key_released(key k, int x, int y)
{
	base::special_key_released(k, x, y);
}

int main(int argc, char * argv[])
{
	app_window w;
	w.start();
	return 0;
}

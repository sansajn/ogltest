#include <chrono>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <boost/filesystem/path.hpp>
#include <glm/vec2.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/shapes.hpp"
#include "gles2/mesh_gles2.hpp"
#include "gles2/label_gles2.hpp"
#include "shadertoy_program.hpp"
#include "file_chooser_dialog.hpp"
#include "delayed_value.hpp"

using std::min;
using std::string;
using std::to_string;
using std::shared_ptr;
using std::cout;
using glm::vec2;
using mesh = gles2::mesh;
using gl::make_quad_xy;
using ui::label;
namespace fs = boost::filesystem;

string const default_shader_program = "hello.glsl";

char const * font_path = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";


string program_directory();

class shadertoy_app : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	shadertoy_app();
	void display() override;
	void input(float dt) override;
	void update(float dt) override;
	void reshape(int w, int h) override;
	bool load_program(string const & fname);
	void edit_program();
	bool reload_program();

private:
	std::chrono::system_clock::time_point _t0;
	bool _open, _edit, _reload, _help;
	delayed_value<bool> _allow_open, _allow_edit, _allow_reload, _allow_help;

	string _program_fname;
	mesh _quad;
	shadertoy_program _prog;
	label _fps_label;
};


void shadertoy_app::update(float dt)
{
	base::update(dt);

	// code there ...
	if (_open)
	{
		if (open_file_chooser_dialog(_program_fname, program_directory()))
			load_program(_program_fname);

		_open = false;
		_allow_open = delayed_value<bool>{false, true, 1.0f};
	}

	if (_edit)
	{
		string cmd = "kate \"" + _program_fname + "\" &";
		system(cmd.c_str());

		_edit = false;
		_allow_edit = delayed_value<bool>{false, true, 1.0f};
	}

	if (_reload)
	{
		if (!_program_fname.empty())
			reload_program();

		_reload = false;
		_allow_reload = delayed_value<bool>{false, true, 0.5f};
	}

	if (_help)
	{
		string cmd = "kate \"help.txt\" &";
		system(cmd.c_str());

		_help = false;
		_allow_help = delayed_value<bool>{false, true, 1.0f};
	}

	// update fps TODO: timer
	static float t = 0.0f;
	float const UPDATE_DELAY = 0.25f;
	t += dt;
	if (t > UPDATE_DELAY)
	{
		_fps_label.text(string("fps: ") + to_string(fps()));
		t = t - UPDATE_DELAY;
	}
}

void shadertoy_app::input(float dt)
{
	base::input(dt);

	_allow_open.update(dt);
	_allow_edit.update(dt);
	_allow_reload.update(dt);

	// code there ...
	if (!_open && _allow_open.get() && in().key('O'))
	{
		cout << "opend dialog ..." << std::endl;
		_open = true;
	}

	if (!_edit && _allow_edit.get() && in().key('E'))
	{
		cout << "edit program ..." << std::endl;
		_edit = true;
	}

	if (!_reload && _allow_reload.get() && in().key('R'))
	{
		cout << "reload program ... " << std::endl;
		_reload = true;
	}

	if (!_help && _allow_help.get() && in().key('H'))
	{
		cout << "show help ..." << std::endl;
		_help = true;
	}
}

void shadertoy_app::reshape(int w, int h)
{
	assert(w > 0 && h > 0 && "invalid screen geometry");
	_fps_label.reshape(vec2{w, h});
	base::reshape(w, h);
}

shadertoy_app::shadertoy_app()
	: base{parameters{}.geometry(400, 300)}
	, _open{false}, _edit{false}, _reload{false}, _help{false}
	, _allow_open{true}
	, _allow_edit{true}
	, _allow_reload{true}
	, _allow_help{true}
{
	// dump help
	cout
		<< "shadertoy [shader_program]\n"
		<< "\n"
		<< "{keys}\n"
		<< "O: open shader program\n"
		<< "R: reload shader program\n"
		<< "E: edit shader program\n"
		<< "H: show this help\n"
		<< std::endl;

	_quad = make_quad_xy<mesh>(vec2{-1,-1}, 2);

	load_program(default_shader_program);

	_fps_label.init(font_path, 12, vec2{width(), height()}, vec2{2,2});

	glClearColor(0,0,0,1);
}

void shadertoy_app::display()
{
	auto now = std::chrono::system_clock::now();
	if (_t0 == std::chrono::system_clock::time_point{})
		_t0 = now;

	std::chrono::duration<double> dt = now - _t0;  // in s

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_prog.update((float)dt.count(), vec2(width(), height()));
	_quad.render();

	// controls
	glDisable(GL_DEPTH_TEST);
	_fps_label.render();

	base::display();
}

void shadertoy_app::edit_program()
{}

bool shadertoy_app::load_program(string const & fname)
{
	_program_fname = fname;
	if (!_prog.load(_program_fname))
		return false;

	_prog.use();
	auto fn = fs::path{_program_fname}.filename();
	name(fn.native());

	cout << "program '" << _program_fname << "' loaded" << std::endl;

	return true;
}

bool shadertoy_app::reload_program()
{
	return load_program(_program_fname);
}

string program_directory()
{
	ssize_t len = 1024;
	char szTmp[32];
	char pBuf[len];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	ssize_t bytes = min(readlink(szTmp, pBuf, len), len - 1);
	if(bytes >= 0)
	  pBuf[bytes] = '\0';

	string result;
	result.assign(pBuf);

	result = result.substr(0, result.find_last_of("/"));  // remove the name of the executable from the end

	return result;
}


int main(int argc, char * argv[])
{
	shadertoy_app app;
	app.start();
	return 0;
}

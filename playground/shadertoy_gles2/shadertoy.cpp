#include <chrono>
#include <algorithm>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <glm/vec2.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/shapes.hpp"
#include "gles2/mesh_gles2.hpp"
#include "shadertoy_program.hpp"
#include "file_chooser_dialog.hpp"

using std::min;
using std::string;
using std::shared_ptr;
using std::cout;
using glm::vec2;
using mesh = gles2::mesh;
using gl::make_quad_xy;
namespace fs = boost::filesystem;

string const default_shader_program = "hello.glsl";

string program_directory();

class shadertoy_app : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	shadertoy_app();
	void display() override;
	void input(float dt) override;
	void update(float dt) override;
	bool load_program(string const & fname);
	void edit_program();
	bool reload_program();

private:
	std::chrono::system_clock::time_point _t0;
	bool _open, _edit, _reload;

	string _program_fname;
	mesh _quad;
	shadertoy_program _prog;
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
	}

	if (_reload)
	{
		if (!_program_fname.empty())
			reload_program();

		_reload = false;
	}
}

void shadertoy_app::input(float dt)
{
	base::input(dt);

	// code there ...
	if (!_open && in().key('O'))
	{
		cout << "opend dialog ..." << std::endl;
		_open = true;
	}

	if (!_edit && in().key('E'))
	{
		cout << "edit program ..." << std::endl;
		_edit = true;
	}

	if (!_reload && in().key('R'))
	{
		cout << "reload program ... " << std::endl;
		_reload = true;
	}
}

shadertoy_app::shadertoy_app()
	: base{parameters{}.geometry(400, 300)}
	, _open{false}, _edit{false}, _reload{false}
{
	_quad = make_quad_xy<mesh>(vec2{-1,-1}, 2);

	load_program(default_shader_program);

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

#include <chrono>
#include <glm/vec2.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/shapes.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"

using std::string;
using std::shared_ptr;
using glm::vec2;
using mesh = gles2::mesh;
using gl::make_quad_xy;
using gles2::shader::uniform;
using gles2::shader::program;

class shadertoy_program
{
public:
	shadertoy_program(string const & fname);
	void use();
	void update(float t, vec2 const & resolution);

private:
	shared_ptr<uniform> _time_u, _resolution_u;
	program _prog;
};

shadertoy_program::shadertoy_program(string const & fname)
{
	string prolog = R"(
		#ifdef _VERTEX_
		attribute vec3 position;
		void main() {
			gl_Position = vec4(position, 1);
		}
		#endif
		#ifdef _FRAGMENT_
		precision mediump float;
		uniform float iTime;
		uniform vec2 iResolution;
	)";

	string mainImage = gles2::shader::read_file(fname);

	string epilog = R"(
		void main() {
			mainImage(gl_FragColor, gl_FragCoord.xy);
		}
		#endif
	)";

	_prog.from_memory(prolog + mainImage + epilog, 100);
}

void shadertoy_program::use()
{
	_prog.use();
	_time_u = _prog.uniform_variable("iTime");
	_resolution_u = _prog.uniform_variable("iResolution");
}

void shadertoy_program::update(float t, vec2 const & resolution)
{
	*_time_u = t;
	*_resolution_u = resolution;
}


class shadertoy_app : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	shadertoy_app();
	void display() override;

private:
	std::chrono::system_clock::time_point _t0;

	mesh _quad;
	shadertoy_program _prog;
};

shadertoy_app::shadertoy_app()
	: _prog{"hello.glsl"}
{
	_quad = make_quad_xy<mesh>(vec2{-1,-1}, 2);

	glClearColor(0,0,0,1);
	_prog.use();
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


int main(int argc, char * argv[])
{
	shadertoy_app app;
	app.start();
	return 0;
}

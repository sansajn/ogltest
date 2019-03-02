#include <chrono>
#include <glm/vec2.hpp>
#include "gl/glfw3_window.hpp"
#include "gl/shapes.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/mesh_gles2.hpp"

using std::shared_ptr;
using glm::vec2;
using mesh = gles2::mesh;
using gl::make_quad_xy;
using gles2::shader::uniform;
using gles2::shader::program;

class shadertoy_app : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	shadertoy_app();
	void display() override;

private:
	std::chrono::system_clock::time_point _t0;

	mesh _quad;
	program _prog;
	shared_ptr<uniform> _time_u, _resolution_u;
};

shadertoy_app::shadertoy_app()
{
	_quad = make_quad_xy<mesh>(vec2{-1,-1}, 2);

	glClearColor(0,0,0,1);

	// load shader program
	_prog.from_file("hello.glsl");
	_prog.use();

	_time_u = _prog.uniform_variable("iTime");
	_resolution_u = _prog.uniform_variable("iResolution");
}

void shadertoy_app::display()
{
	auto now = std::chrono::system_clock::now();
	if (_t0 == std::chrono::system_clock::time_point{})
		_t0 = now;

	std::chrono::duration<double> dt = now - _t0;  // in s

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	*_time_u = (float)dt.count();
	*_resolution_u = vec2(width(), height());
	_quad.render();
	base::display();
}


int main(int argc, char * argv[])
{
	shadertoy_app app;
	app.start();
	return 0;
}

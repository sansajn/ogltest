// utilita k vyvoju shadera pre povistrelovy blast efekt
#include <memory>
#include <chrono>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "gl/program.hpp"
#include "gl/glut_window.hpp"

char const * blast_shader_path = "blast.glsl";

using std::shared_ptr;
using glm::vec2;
using gl::mesh;
using shader::program;
using shader::uniform;
using gl::make_quad_xy;


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void display() override;
	void update(float dt) override;

private:
	using hres_clock = std::chrono::high_resolution_clock;

	program _blast;
	mesh _quad;
	shared_ptr<uniform> _resolution_u;
	shared_ptr<uniform> _global_time_u;
	float _global_time = 0.f;
};

scene_window::scene_window()
{
	_blast.from_file(blast_shader_path);
	_quad = make_quad_xy<mesh>(vec2{-1,-1}, 2.f);
	_resolution_u = _blast.uniform_variable("resolution");
	_global_time_u = _blast.uniform_variable("global_time");
	glClearColor(0,0,0,1);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	_blast.use();
	*_resolution_u = vec2(width(), height());
	*_global_time_u = _global_time;
	_quad.render();
	base::display();
}

void scene_window::update(float dt)
{
	base::update(dt);
	_global_time += dt;
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

// test pouzitia ortho_label
#include <string>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "gl/shapes.hpp"
#include "gl/colors.hpp"
#include "gl/glfw3_window.hpp"
#include "gl/free_camera.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/mesh_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"
#include "gles2/shaded_shader.hpp"
#include "gles2/label_gles2.hpp"

using std::string;
using std::to_string;
using glm::mat4;
using glm::vec3;
using glm::mat3;
using glm::vec2;
using glm::translate;
using glm::rotate;
using glm::scale;
using glm::inverseTranspose;
using glm::normalize;
using gl::shape_generator;
using gles2::mesh;

char const * font_path = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

class scene_window : public ui::glfw_pool_window
{
public:
	using base = ui::glfw_pool_window;

	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;
	void reshape(int w, int h) override;

private:
	mesh _cube, _box, _sphere, _quad;
	ui::label _lbl;
	gl::free_camera<scene_window> _view;
	gles2::shaded_shader _shaded;
};

scene_window::scene_window()
	: _view{glm::radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	shape_generator<mesh> shape;
	_cube = shape.cube();
	_box = shape.box(vec3{.5, 1, .5});
	_sphere = shape.sphere(.5);
	_quad = shape.quad_xy();
	_lbl.init(font_path, 12, vec2{width(), height()}, glm::vec2{2,2});
	_lbl.text("hello!");
	_view.get_camera().position = vec3{3,3,3};
	_view.get_camera().look_at(vec3{0,0,0});
	glClearColor(0,0,0,1);
	_shaded.light_direction(normalize(vec3{1,2,3}));
}

void scene_window::update(float dt)
{
	base::update(dt);

	// update fps TODO: timer
	static float t = 0.0f;
	t += dt;
	if (t > 1.0f)
	{
		_lbl.text(string("fps: ") + to_string(fps()));
		t = t - 1.0f;
	}
}

void scene_window::display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_shaded.view_projection(_view.get_camera().view_projection());

	// cube
	mat4 M{1};
	_shaded.object_color(rgb::lime);
	_shaded.use();
	_shaded.update_properties();
	_shaded.render(_cube, M, mat3{inverseTranspose(M)});

	// box
	M = translate(vec3{2, 0, -2});
	_shaded.object_color(rgb::purple);
	_shaded.update_properties();
	_shaded.render(_box, M, mat3{inverseTranspose(M)});

	// sphere
	M = translate(vec3{2, 0, 1});
	_shaded.object_color(rgb::cyan);
	_shaded.update_properties();
	_shaded.render(_sphere, M, mat3{inverseTranspose(M)});

	_lbl.render();

	base::display();
}

void scene_window::input(float dt)
{
	_view.input(dt);
	base::input(dt);
}

void scene_window::reshape(int w, int h)
{
	assert(w > 0 && h > 0 && "invalid screen geometry");
//	_lbl.reshape(vec2{w, h});
	base::reshape(w, h);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

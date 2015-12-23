// sprite animacia
#include <string>
#include <boost/logic/tribool.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "gl/glut_window.hpp"
#include "gl/scene_object.hpp"
#include "gl/controllers.hpp"
#include "gl/sprite_model.hpp"
#include "gl/shapes.hpp"
#include "gl/texture_loader.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::translate;
using glm::radians;

string const checker_texture_path = "assets/textures/checker16x16.png";

string const textured_shader_source = R"(
	// zobrazi otexturovany model (bez osvetlenia)
	uniform mat4 local_to_screen;
	uniform sampler2D s;  // diffuse texture sampler
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 texcoord;
	out vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = texture(s, uv);
	}
	#endif
)";


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	~scene_window();
	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	gl::free_camera<scene_window> _view;
	shader::program _sprite_prog;
	axis_object _axis;
	sprite_model * _model;
	shader::program _textured_prog;
	texture2d _checker_tex;
	gl::mesh _plane;
	boost::tribool _death = false;
};

scene_window::scene_window()
	: _view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_view.get_camera().position = vec3{0,1,3};
	_sprite_prog.from_memory(default_sprite_model_shader_source);

	std::string files[] = {
		"assets/textures/sswvi0.png",
		"assets/textures/sswvj0.png",
		"assets/textures/sswvk0.png",
		"assets/textures/sswvl0.png",
		"assets/textures/sswvm0.png"};

	_model = new sprite_model{files, 5, 64, 64};
	_model->frame_rate(5);

	_textured_prog.from_memory(textured_shader_source);
	_checker_tex = gl::texture_from_file(checker_texture_path, texture::parameters{}.filter(texture_filter::nearest, texture_filter::nearest));
	_plane = gl::make_plane_xz<gl::mesh>(10, 10, 10);

	glClearColor(0,0,0,1);
}

scene_window::~scene_window()
{
	delete _model;
}

void scene_window::input(float dt)
{
	if (in.key(' '))
		_death = true;

	_view.input(dt);
	base::input(dt);
}

void scene_window::update(float dt)
{
	base::update(dt);

	if (_death)
	{
		_model->animation_sequence(0, 5);
		_death = boost::indeterminate;
	}

	_model->update(dt);
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	mat4 world_to_screen = _view.get_camera().view_projection();

	_textured_prog.use();
	mat4 local_to_screen = world_to_screen * translate(vec3{-5, 0, 5});
	_textured_prog.uniform_variable("local_to_screen", local_to_screen);
	_checker_tex.bind(0);
	_textured_prog.uniform_variable("s", 0);
	_plane.render();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	_sprite_prog.use();
	local_to_screen = world_to_screen * translate(vec3{0,1,0});
	_sprite_prog.uniform_variable("local_to_screen", local_to_screen);
	_model->render(_sprite_prog);

	_axis.render(world_to_screen);

	base::display();
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

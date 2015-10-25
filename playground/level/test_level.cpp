// renderuje steny, strop a dlazku levelu
#include <vector>
#include <string>
#include <glm/gtc/matrix_inverse.hpp>
#include "window.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "scene_object.hpp"
#include "level.hpp"
#include "player.hpp"

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using gl::mesh;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	level _lvl;
	player _player;
	axis_object _axis;
	light_object _light;
};

scene_window::scene_window()
	: _player{*this}
{
	// TODO: treba volat v spravnom poradi
	view_properties v;
	v.fovy = radians(70.0);
	v.aspect_ratio = aspect_ratio();
	v.near = 0.01;
	v.far = 1000.0;
	_player.view(v);
	_player.position(_lvl.player_position());

	glClearColor(0, 0, 0, 1);
}

void scene_window::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_lvl.render(_player.get_camera());
	_axis.render(_player.get_camera());
	_light.render(_player.get_camera(), light_pos);

	base::display();
}

void scene_window::input(float dt)
{
	_player.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

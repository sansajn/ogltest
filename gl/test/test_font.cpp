// do opengl okna vyrenderuje text
#include <string>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/glut_window.hpp"
#include "gl/label.hpp"
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/camera.hpp"
#include "gl/model_loader.hpp"

using std::string;
using std::to_string;
using gl::mesh;
using gl::mesh_from_file;
using gl::camera;

string const font_path = "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf";

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;
	scene_window();
	void display() override;
	void update(float dt) override;

private:
	mesh _monkey;
	shader::program _prog;
	camera _cam;
	ui::label<scene_window> _fps_label;
	float _yrot;
	GLuint _vao;
};

scene_window::scene_window()
	: base(parameters().name("fps counter test"))
{
	_monkey = mesh_from_file("assets/models/monkey.obj");
	_prog.from_file("assets/shaders/test_font_phong.glsl");

	_fps_label.init(0, 0, *this);
	_fps_label.font(font_path, 12);
	_fps_label.text("fps:0");

	_cam = camera{glm::vec3{1,1,5}, glm::radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.look_at(glm::vec3{0,0,0});

	_yrot = 0.0f;

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
}

void scene_window::update(float dt)
{
	base::update(dt);

	_yrot += glm::radians(60*dt);

	static float time_count = 0;
	time_count += dt;

	if (time_count > 1.0f)
	{
		_fps_label.text(string("fps:") + to_string(fps()));
		time_count = 0.0f;
	}
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_prog.use();

	// [transformations]
	glm::mat4 M = glm::mat4(1);
	M = glm::rotate(M, _yrot, glm::vec3{0,1,0});
	glm::mat4 V = _cam.world_to_camera();
	glm::mat4 P = _cam.projection();
	glm::mat4 local_to_camera = V*M;
	glm::mat4 local_to_screen = P*local_to_camera;
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("local_to_camera", local_to_camera);

	// [render]
	_monkey.render();

	_fps_label.render();

	base::display();
}

int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	std::cout << "done!" << std::endl;
	return 0;
}

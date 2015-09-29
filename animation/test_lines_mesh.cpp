#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "window.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "program.hpp"
#include "mesh_exp.hpp"

using std::shared_ptr;
using std::vector;
using std::string;
using glm::vec3;
using glm::mat4;
using glm::mat3;
using glm::radians;

string solid_shader_path = "solid.glsl";

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	gl::camera _cam;
	vector<shared_ptr<gl::camera_controller>> _cam_ctrls;
	shader::program _solid_prog;
	gl::mesh _skeleton;
};

gl::mesh create_dummy_skeleton_mesh()
{
	// positions
	vector<vec3> positions{
		vec3{0,0,0}, vec3{1,0,0}, vec3{1,1,0}, vec3{0,1,0}};

	vector<unsigned> indices{0,1, 1,3, 3,2, 2,0};

	gl::mesh m(positions.data(), positions.size()*sizeof(vec3), indices.data(), indices.size());
	m.append_attribute(gl::attribute{0, 3, GL_FLOAT, 3*sizeof(GL_FLOAT)});
	m.draw_mode(GL_LINES);

	return m;
}

scene_window::scene_window()
	: _cam{radians(70.0f), aspect_ratio(), 0.01, 1000}
{
	_skeleton = create_dummy_skeleton_mesh();
	_cam.position = vec3{0,3,10};
	_cam_ctrls.push_back(shared_ptr<gl::free_move<scene_window>>{new gl::free_move<scene_window>{_cam, *this, 0.1}});
	_cam_ctrls.push_back(shared_ptr<gl::free_look<scene_window>>{new gl::free_look<scene_window>{_cam, *this}});
	_solid_prog.from_file(solid_shader_path);

	glClearColor(0,0,0,1);
}

void scene_window::display()
{
	mat4 M = mat4{1};
	mat4 local_to_screen = _cam.view_projection() * M;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// zobraz skeleton
	_solid_prog.use();
	_solid_prog.uniform_variable("local_to_screen", local_to_screen);
	_solid_prog.uniform_variable("color", vec3{1,0,0});
	_skeleton.render();

	base::display();
}

void scene_window::input(float dt)
{
	for (auto ctrl : _cam_ctrls)
		ctrl->input(dt);

	base::input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

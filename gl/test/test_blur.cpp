// blur filter aplykovany na texturu
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "gl/mesh.hpp"
#include "gl/shapes.hpp"
#include "gl/texture_loader.hpp"
#include "gl/glut_window.hpp"

char const * picture_name = "../../assets/textures/lena.png";

using std::cout;
using std::string;
using std::make_pair;
using gl::mesh;
using gl::make_quad_xy;
using ui::glut_window;

float normpdf(float x, float sigma);

char const * blur_shader_source = R"(
	// Dvojpriechodova implementacia gausovho filtra.
	uniform sampler2D tex;
	uniform vec2 direction;  // vec2(dx,0) or vec2(0,dy)
	uniform float weights[5];
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texcoord;
	out vec2 uv;
	void main()	{
		uv = texcoord;
		gl_Position = vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		vec2 uv = uv;
		vec4 sum = vec4(0,0,0,0);
		sum += texture(tex, uv + direction*-4) * weights[4];
		sum += texture(tex, uv + direction*-3) * weights[3];
		sum += texture(tex, uv + direction*-2) * weights[2];
		sum += texture(tex, uv - direction) * weights[1];
		sum += texture(tex, uv) * weights[0];
		sum += texture(tex, uv + direction) * weights[1];
		sum += texture(tex, uv + direction*2) * weights[2];
		sum += texture(tex, uv + direction*3) * weights[3];
		sum += texture(tex, uv + direction*4) * weights[4];
		fcolor = sum;
	}
	#endif
)";

class main_window : public glut_window
{
public:
	using base = glut_window;

	main_window();
	void display() override;

private:
	void compute_filter_weights();

	texture2d _srctex;
	texture2d _fbtex1;
	texture2d _fbtex2;
	mesh _texframe;
	shader::program _viewprog;
	shader::program _blurprog;
	float _weights[5];  // normpdf weights
	float _sigma;
};

main_window::main_window()
	: base{parameters().name("blur example")}
{
	_srctex = gl::texture_from_file(picture_name);
	int w = _srctex.width(), h = _srctex.height();
	_fbtex1 = texture2d(w, h);
	_fbtex2 = texture2d(w, h);
	_texframe = make_quad_xy<mesh>();

	_blurprog.from_memory(blur_shader_source);

	std::shared_ptr<shader::module> view_module(new shader::module("assets/shaders/show.glsl"));
	_viewprog.attach(view_module);

	_sigma = 2.0f;
}

void main_window::display()
{
	// filter phase
	_blurprog.use();
	_fbtex1.bind_as_render_target();
	_srctex.bind(0);
	_blurprog.uniform_variable("tex", 0);

	// weights
	compute_filter_weights();
	_blurprog.uniform_variable("weights", make_pair(_weights, 5));

	// x direction filtering
	_blurprog.uniform_variable("direction", glm::vec2(1.0f/_srctex.width(), 0.0f));
	_texframe.render();

	// y direction filtering
	_fbtex2.bind_as_render_target();
	_fbtex1.bind(0);
	_blurprog.uniform_variable("direction", glm::vec2(1.0f/_srctex.height(), 0.0f));

	_texframe.render();

	texture & resulttex = _fbtex2;

	// show phase
	glm::mat4 V(1);
	V = glm::scale(V, glm::vec3(1/aspect_ratio(), 1, 1));
	_viewprog.use();
	bind_as_render_target();
	resulttex.bind(0);
	_viewprog.uniform_variable("tex", 0);
	_viewprog.uniform_variable("transform", V);
	_texframe.render();

	base::display();
}

void main_window::compute_filter_weights()
{
	_weights[0] = normpdf(0, _sigma);
	float sum = _weights[0];
	for (int i = 1; i < 5; ++i)
	{
		_weights[i] = normpdf(i, _sigma);
		sum += 2*_weights[i];
	}

	for (int i = 0; i < 5; ++i)
		_weights[i] /= sum;
}

float normpdf(float x, float sigma)
{
	float k = x/sigma;
	return exp(-0.5f*k*k) / sqrt(2*M_PI)*sigma;
}

int main(int argc, char * argv[])
{
	main_window w;
	w.start();
	return 0;
}

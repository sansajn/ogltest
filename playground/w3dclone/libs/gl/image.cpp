#include "image.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "mesh.hpp"
#include "shapes.hpp"
#include "texture_loader.hpp"

namespace ui {

using std::string;
using gl::mesh;
using gl::make_quad_xy;

char const * shader_source = R"(
	// #version 330
	uniform sampler2D s;  // image to render
	uniform mat4 T = mat4(1);  // image transform
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;  // ocakava obdlznik [-1,-1,1,1]
	out vec2 st;
	void main() {
	  st = position.xy*0.5 + 0.5;
	  gl_Position = T * vec4(position.xy, 0, 1);
	}
	#endif  // _VERTEX_
	#ifdef _FRAGMENT_
	in vec2 st;
	out vec4 fcolor;
	void main() {
	  fcolor = texture(s, st);
	}
	#endif  // _FRAGMENT_
)";


image::image(std::string const & file_name)
	: _t{1}
{
	_prog.from_memory(shader_source);
	_sampler_u = _prog.uniform_variable("s");
	_transform_u = _prog.uniform_variable("T");
	_image_tex = gl::texture_from_file(file_name);
}

void image::render()
{
	_prog.use();
	*_sampler_u = 0;
	_image_tex.bind(0);
	*_transform_u = _t;

	static mesh quad = make_quad_xy<mesh>();
	quad.render();
}

void image::transform(glm::mat4 const & t)
{
	_t = t;
}

}  // ui

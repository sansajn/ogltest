#include "image.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "mesh.hpp"

namespace ui {

std::string const shader_source{
	"uniform sampler2D s;  // image to render\n\
	uniform mat4 t = mat4(1);  // image transform\n\
	\
	#ifdef _VERTEX_\n\
	\
	layout(location = 0) in vec3 position;  // ocakava obdlznik [-1,-1,1,1]\n\
	out vec2 st;\n\
	void main() {\n\
		st = position.xy*0.5 + 0.5;\n\
		gl_Position = t * vec4(position.xy, 0, 1);\n\
	}\n\
	\
	#endif  // _VERTEX_\n\
	#ifdef _FRAGMENT_\n\
	\
	in vec2 st;\n\
	out vec4 fcolor;\n\
	void main() {\n\
		fcolor = texture(s, st);\n\
	}\n\
	\
	#endif  // _FRAGMENT_\n"
};

image::image(std::string const & file_name)
	: _t{1}
{
	_prog.from_memory(shader_source);
	_sampler_u = _prog.uniform_variable("s");
	_transform_u = _prog.uniform_variable("t");
	_image_tex = texture2d{file_name};
}

void image::render()
{
	_prog.use();
	*_sampler_u = 0;
	_image_tex.bind(0);
	*_transform_u = _t;

	static mesh quad = make_quad_xy();
	quad.draw();
}

void image::transform(glm::mat4 const & t)
{
	_t = t;
}

}  // ui

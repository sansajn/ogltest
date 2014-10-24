// použitie array textury (zobrazi len druhu texturu)
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "core/ptr.hpp"
#include "render/types.h"
#include "render/program.hpp"
#include "render/mesh.h"
#include "render/framebuffer.h"
#include "ui/sdl_window.h"
#include "render/texture2d_array.h"
#include "render/cpubuffer.h"
#include "render/sampler_uniform.h"

class minimal_example : public sdl_window
{
public:
	minimal_example() 
		: sdl_window(parameters().size(800, 600).version(3, 3)), _fb(framebuffer::default_fb())
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);  // TODO tieto dva riadky odstran (niekto ich nastavuje nejako)

		_m = make_ptr<mesh<glm::vec2, unsigned int>>(mesh_mode::triangle_strip,
			mesh_usage::GPU_STATIC);
		_m->append_attribute_type(0, 2, attribute_type::f32, false);
		_m->append_vertex(glm::vec2(-1, -1));
		_m->append_vertex(glm::vec2(1, -1));
		_m->append_vertex(glm::vec2(-1, 1));
		_m->append_vertex(glm::vec2(1, 1));

		_p = make_ptr<shader_program>();
		*_p << "shader/minimal_array.fs";
		_p->link();
		_p->use();
		_u = make_ptr<sampler_uniform>("sampler", _p);

		uniform_variable u_scale("scale", *_p);
		u_scale = glm::vec2(1.0/800, 1.0/600);

		uniform_variable u_layer("layer", *_p);
		u_layer = 1;

		uint8_t data[32] = {
			0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
			255, 255, 255, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255};

		ptr<texture2D_array> tex(new texture2D_array(4, 4, 2, texture_internal_format::R8,
			texture_format::RED, pixel_type::UNSIGNED_BYTE, texture::parameters().mag(texture_filter::NEAREST),
			buffer::parameters(), cpubuffer(data)));

		*_u = tex;
	}

	void display() override
	{
		_fb.clear(true, true);
		_fb.draw(*_p, *_m);
		sdl_window::display();
	}

	void reshape(int x, int y) override
	{
		_fb.viewport(glm::ivec4(0, 0, x, y));
		uniform_variable u_scale("scale", *_p);
		u_scale = glm::vec2(1.0f/x, 1.0f/y);
		sdl_window::reshape(x, y);
	}

private:
	ptr<shader_program> _p;
	ptr<mesh<glm::vec2, unsigned int>> _m;
	ptr<sampler_uniform> _u;
	framebuffer & _fb;
	GLuint _vao;
};


int main(int argc, char * argv[])
{
	ptr<minimal_example> app(new minimal_example());
	app->start();
	return 0;
}

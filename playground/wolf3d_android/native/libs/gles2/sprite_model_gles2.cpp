#include "sprite_model_gles2.hpp"
#include <memory>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include "gl/shapes.hpp"

using std::move;
using std::unique_ptr;
using std::string;
using gles2::mesh;
using gles2::shader::program;
using gl::make_quad_xy;
using namespace boost::gil;

sprite_model::sprite_model(std::string files[], int n, int width, int height)
{
	_first = _cur = 0;
	_last = 1;
	_sprite_count = n;
	_mode = repeat_mode::once;
	_mesh = make_quad_xy<mesh>();

	rgba8_image_t im{width, height};

	for (int i = 0; i < n; ++i)
	{
		rgba8_image_t tmp;
		png_read_image(files[i], tmp);
		rgba8_view_t src = flipped_up_down_view(view(tmp));

		fill_pixels(view(im), rgba8_pixel_t{0,0,0,0});
		int x = (width-tmp.width()) / 2;
		rgba8_view_t dst = subimage_view(view(im), x, 0, tmp.width(), tmp.height());
		copy_pixels(src, dst);

		void * pixels = (void *)&(*view(im).begin());
		_sprites.emplace_back(width, height, gles2::pixel_format::rgba, gles2::pixel_type::ub8, pixels, gles2::texture::parameters{}.filter(gles2::texture_filter::nearest));
	}
}

void sprite_model::update(float dt)
{
	_t += dt;
	if (_t >= _period)
	{
		++_cur;
		if (_cur == _last)
			_cur = _mode == repeat_mode::once ? _last-1 : _first;
		_t = _t - _period;
		assert(_t >= 0 && "zaporny cas");
	}
}

void sprite_model::render(program & p)
{
	_sprites[_cur].bind(0);
	p.uniform_variable("s", 0);
	_mesh.render();
}

void sprite_model::animation_sequence(int first, int last, repeat_mode m)
{
	_first = first;
	_last = last;
	_cur = first;
	_mode = m;
	_t = 0;
	assert(last <= _sprite_count && "out of range");
}

void sprite_model::operator=(sprite_model && other)
{
	_first = other._first;
	_last = other._last;
	_cur = other._cur;
	_sprite_count = other._sprite_count;
	_mode = other._mode;
	_mesh = move(other._mesh);
	_sprites = move(other._sprites);
	_period = other._period;
	_t = other._t;
}


//! texturovany model, bez osvetlenia (gles2 nema pole textur)
char const * default_sprite_model_shader_source = R"(
	#ifdef _VERTEX_
	uniform mat4 local_to_screen;
	attribute vec3 position;
	attribute vec2 texcoord;
	varying vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform sampler2D s;
	varying vec2 uv;
	void main() {
		gl_FragColor = texture2D(s, uv);
	}
	#endif
)";

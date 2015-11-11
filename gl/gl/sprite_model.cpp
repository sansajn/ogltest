#include "sprite_model.hpp"
#include <memory>
#include <Magick++.h>

using std::unique_ptr;
using std::string;

// sprite helpers

static void copy32(void const * src, int srcw, int srch, int dstx, int dsty, int dstw, int dsth, void * dst)
{
	uint32_t const * s = (uint32_t const *)src;
	for (int r = 0; r < srch; ++r)
	{
		uint32_t * d = (uint32_t *)dst + ((r+dsty) * dstw + dstx);
		for (int c = 0; c < srcw; ++c)
			*(d++) = *(s++);
	}
}

sprite_model::sprite_model(std::string files[], int n, int width, int height)
{
	_first = _cur = 0;
	_last = 1;
	_sprite_count = n;
	_mode = repeat_mode::once;
	_mesh = gl::make_quad_xy();

	// postupne citaj subory a ich obsah kopiruj do jedneho buffra
	int size = n*width*height*4;  // RGBA
	unique_ptr<uint8_t []> data{new uint8_t[size]};
	memset(data.get(), 0, size);

	for (int i = 0; i < n; ++i)
	{
		Magick::Image im{files[i].c_str()};
		im.flip();
		Magick::Blob imblob;
		im.write(&imblob, "RGBA");
		int x = (width-im.columns())/2;
		uint8_t * dst = data.get() + width*height*i*4;
		copy32(imblob.data(), im.columns(), im.rows(), x, 0, width, height, dst);
	}

	_sprites = texture2d_array(width, height, n, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, data.get(), texture::parameters{}.filter(texture_filter::nearest));
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

void sprite_model::render(shader::program & p)
{
	_sprites.bind(0);
	p.uniform_variable("sprites", 0);
	p.uniform_variable("sprite_idx", _cur);
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

string const default_sprite_model_shader_source = R"(
	// zobrazi otexturovany model (bez osvetlenia)
	uniform mat4 local_to_screen;
	uniform sampler2DArray sprites;
	uniform int sprite_idx = 0;
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
		fcolor = texture(sprites, vec3(uv, sprite_idx));
	}
	#endif
)";

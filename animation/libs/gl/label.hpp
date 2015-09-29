#pragma once
#include <string>
#include <vector>
#include <map>
#include <glm/vec4.hpp>
#include <Magick++.h>
#include <ft2build.h>
#include FT_GLYPH_H
#include FT_FREETYPE_H
#include "program.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "window.hpp"

namespace ui {

/*! Label control implementation.
\code
	label l{0, 0, window};  // left-top corner
	l.font("arial.ttf", 24);
	l.text("Teresa Lisbon");
\note class is moveable */
template <typename Window>
class label
{
public:
	label();  //!< \note not valid after creation \sa init
	label(unsigned x, unsigned y, Window const & parent);
	~label();

	void init(unsigned x, unsigned y, Window const & parent);

	void text(std::string const & s);
	void position(unsigned x, unsigned y);
	void font(std::string const & file_name, unsigned size);

	void show() {_visible = true;}  // TODO: toto je zakladna vlastnost kazdeho ui prvku
	void show(bool b) {_visible = b;}
	void hide() {_visible = false;}
	bool visible() const {return _visible;}

	unsigned width() const {return _text_tex.width();}
	unsigned height() const {return _text_tex.height();}

	void render();

	label(label const &) = delete;
	label & operator=(label const &) = delete;

private:
	unsigned const dpi = 96;  // TODO: custom dpi

	void build_text_texture();
	std::vector<FT_Glyph> load_glyphs(std::string const & s);

	unsigned _x, _y;
	std::string _text;
	Window const * _win;

	texture2d _text_tex;
	shader::program _text_prog;
	FT_Library _lib;
	FT_Face _face;
	std::map<unsigned, FT_Glyph> _cache;

	bool _visible;
};  // label

namespace detail {

std::string const text_shader{
	"// shader renderujuci text (ocakava stvorec [-1,-1,1,1])\n\
	#ifdef _VERTEX_\n\
	layout(location = 0) in vec3 position;\n\
	uniform vec4 os = vec4(0, 0, 1, 1);  // (offset.xy, scale.xy)\n\
	out vec2 st;\n\
	void main()	{\n\
		st = vec2(position.x, -position.y)/2.0 + 0.5;  // flip\n\
		gl_Position = vec4(os.xy + os.zw*position.xy, 0 , 1);\n\
	}\n\
	#endif  // _VERTEX_\n\
	#ifdef _FRAGMENT_\n\
	uniform sampler2D s;\n\
	in vec2 st;\n\
	out vec4 fcolor;\n\
	void main()	{\n\
		fcolor = vec4(texture(s, st).rrr, 1);\n\
	}\n\
	#endif  // _FRAGMENT_\n"
};

Magick::Image render_glyphs(std::vector<FT_Glyph> const & glyphs);
FT_Glyph load_glyph(unsigned char_code, FT_Face face);

}  // detail

template <typename Window>
label<Window>::label() : _win(nullptr), _lib(nullptr), _face(nullptr)
{}

template <typename Window>
label<Window>::label(unsigned x, unsigned y, Window const & parent)
	: _win(nullptr), _lib(nullptr), _face(nullptr)
{
	init(x, y, parent);
}

template <typename Window>
void label<Window>::init(unsigned x, unsigned y, Window const & parent)
{
	assert(!_win && "already initialized");

	_x = x;
	_y = y;
	_win = &parent;
	_visible = true;

	// [initialize freetype]
	FT_Error err = FT_Init_FreeType(&_lib);
	assert(!err && "unable to init a free-type library");

	_text_prog.from_memory(detail::text_shader);
}

template <typename Window>
void label<Window>::font(std::string const & file_name, unsigned size)
{
	FT_Error err = FT_New_Face(_lib, file_name.c_str(), 0, &_face);
	assert(!err && "unable to load font face");

	unsigned font_size = size << 6;  // 26.6
	err = FT_Set_Char_Size(_face, font_size, font_size, dpi, dpi);
	assert(!err && "freetype set font size failed");

	if (!_text.empty())
		build_text_texture();
}

template <typename Window>
label<Window>::~label()
{
	for (auto e : _cache)
		FT_Done_Glyph(e.second);

	FT_Done_Face(_face);
	FT_Done_FreeType(_lib);
}

template <typename Window>
void label<Window>::render()
{
	if (!_visible || _text.empty())
		return;

	assert(_win && "label not initialized (use init() first)");

	_text_prog.use();
	_text_prog.uniform_variable("s", 0);

	float win_w = _win->width(), win_h = _win->height();
	glm::vec2 offset{-1 + width()/win_w + 2*(_x/win_w), 1 - height()/win_h - 2*(_y/win_h)};
	glm::vec2 scale{_text_tex.width()/win_w, _text_tex.height()/win_h};
	_text_prog.uniform_variable("os", glm::vec4{offset.x, offset.y, scale.x, scale.y});

	_text_tex.bind(0);

	static mesh quad = make_quad_xy();
	quad.draw();
}

template <typename Window>
void label<Window>::text(std::string const & s)
{
	assert(_face && "set font first");

	if (_text == s)
		return;

	_text = s;
	build_text_texture();
}

template <typename Window>
void label<Window>::position(unsigned x, unsigned y)
{
	_x = x;
	_y = y;
}

template <typename Window>
void label<Window>::build_text_texture()
{
	if (_text.empty())
		return;

	std::vector<FT_Glyph> glyphs = load_glyphs(_text);
	Magick::Image im = detail::render_glyphs(glyphs);
	Magick::Blob blob;
	im.write(&blob, "GRAY");

	_text_tex = texture2d(im.columns(), im.rows(), sized_internal_format::r8,
		pixel_format::red, pixel_type::ub8, blob.data());
}

template <typename Window>
std::vector<FT_Glyph> label<Window>::load_glyphs(std::string const & s)
{
	std::vector<FT_Glyph> result;
	result.reserve(s.size());

	for (auto char_code : s)
	{
		auto glyph_it = _cache.find(char_code);
		if (glyph_it != _cache.end())
			result.push_back(glyph_it->second);
		else
		{
			FT_Glyph glyph = detail::load_glyph(char_code, _face);
			_cache[char_code] = glyph;
			result.push_back(glyph);
		}
	}

	return result;
}

}  // ui

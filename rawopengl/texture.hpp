#pragma once
#include <string>

enum class pixel_type {  //!< \sa glTexImage2D:type
	ub8,
	b8,
	us16,
	s16,
	ui32,
	i32,
	f32,
	ub332,
	ub233r,
	us565,
	us565r,
	us4444,
	us4444r,
	us5551,
	us1555r,
	ui8888,
	ui8888r,
	ui1010102,
	ui2101010r
};

enum class pixel_format {  //!< \sa glTexImage2D:format
	red,
	rg,
	rgb,
	bgr,
	rgba,
	bgra,
	redi,
	rgi,
	rgbi,
	bgri,
	rgbai,
	bgrai,
	stencil_index,
	depth_component,
	depth_stencil
};

enum class base_internal_format  //!< \sa glTexImage2D:internalFormat
{
	depth_component,
	depth_stencil,
	red,
	rg,
	rgb,
	rgba,
};

enum class sized_internal_format  //!< \sa glTexImage2D:internalFormat
{
	r8,
	r8_snorm,
	r16,
	r16_snorm,
	rg8,
	rg8_snorm,
	rg16,
	rg16_snorm,
	r3g3b2,
	rgb4,
	rgb5,
	rgb8,
	rgb8_snorm,
	rgb10,
	rgb12,
	rgb16_snorm,
	rgba2,
	rgba4,
	rgb5_a1,
	rgba8,
	rgba8_snorm,
	rgb10_a2,
	rgb10_a2ui,
	rgba12,
	rgba16,
	srgb8,
	srgb8_alpha8,
	r16f,
	rg16f,
	rgb16f,
	rgba16f,
	r32f,
	rg32f,
	rgb32f,
	rgba32f,
	r11f_g11f_b10f,
	rgb9_e5,
	r8i,
	r8ui,
	r16i,
	r16ui,
	rg32i,
	rg32ui,
	rgb8i,
	rgb8ui,
	rgb16i,
	rgb16ui,
	rgb32i,
	rgb32ui,
	rgba8i,
	rgba8ui,
	rgba16i,
	rgba16ui,
	rgba32i,
	rgba32ui
};

enum class compressed_internal_format  //!< \sa glTexImage2D:internalFormat
{
	compressed_red,
	compressed_rg,
	compressed_rgb,
	compressed_rgba,
	compressed_srgb,
	compressed_srgb_alpha,
	compressed_red_rgtc1,
	compressed_signed_red_rgtc1,
	compressed_rg_rgtc2,
	compressed_signed_rg_rgtc2,
	compressed_rgba_bptc_unorm,
	compressed_srgb_alpha_bptc_unorm,
	compressed_rgb_bptc_signed_float,
	compressed_rgb_bptc_unsigned_float
};

enum class internal_format {  //!< \sa glTexImage2D:internalFormat
	// base
	depth_component,
	depth_stencil,
	red,
	rg,
	rgb,
	rgba,

	// sized
	r8,
	r8_snorm,
	r16,
	r16_snorm,
	rg8,
	rg8_snorm,
	rg16,
	rg16_snorm,
	r3g3b2,
	rgb4,
	rgb5,
	rgb8,
	rgb8_snorm,
	rgb10,
	rgb12,
	rgb16_snorm,
	rgba2,
	rgba4,
	rgb5_a1,
	rgba8,
	rgba8_snorm,
	rgb10_a2,
	rgb10_a2ui,
	rgba12,
	rgba16,
	srgb8,
	srgb8_alpha8,
	r16f,
	rg16f,
	rgb16f,
	rgba16f,
	r32f,
	rg32f,
	rgb32f,
	rgba32f,
	r11f_g11f_b10f,
	rgb9_e5,
	r8i,
	r8ui,
	r16i,
	r16ui,
	rg32i,
	rg32ui,
	rgb8i,
	rgb8ui,
	rgb16i,
	rgb16ui,
	rgb32i,
	rgb32ui,
	rgba8i,
	rgba8ui,
	rgba16i,
	rgba16ui,
	rgba32i,
	rgba32ui,

	// compressed
	compressed_red,
	compressed_rg,
	compressed_rgb,
	compressed_rgba,
	compressed_srgb,
	compressed_srgb_alpha,
	compressed_red_rgtc1,
	compressed_signed_red_rgtc1,
	compressed_rg_rgtc2,
	compressed_signed_rg_rgtc2,
	compressed_rgba_bptc_unorm,
	compressed_srgb_alpha_bptc_unorm,
	compressed_rgb_bptc_signed_float,
	compressed_rgb_bptc_unsigned_float
};  // internal_format

class texture
{
public:
	texture();
	texture(std::string const & fname);
	texture(unsigned width, unsigned height, sized_internal_format ifmt = sized_internal_format::rgba8);
	texture(unsigned tid, unsigned width, unsigned height, pixel_format pfmt, pixel_type type);
	texture(unsigned width, unsigned height, void * pixels) : texture(width, height, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, pixels) {}
	texture(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void * pixels);
	texture(texture && lhs);
	~texture();

	void bind(unsigned unit);
	void bind_as_render_target(bool depth = true);
	void write(std::string const & fname);

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	unsigned id() const {return _tid;}

	void operator=(texture && lhs);

	texture(texture const &) = delete;
	void operator=(texture const &) = delete;

private:
	void create_framebuffer();
	void create_depthbuffer();
	void read(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void * data);

	unsigned _tid;  //!< texture identifier
	unsigned _fid;  //!< framebuffer identifier
	unsigned _rid;  //!< renderbuffer identifier
	unsigned _w, _h;
	pixel_format _fmt;
	pixel_type _type;
};

class texture_array
{
public:
	texture_array() : _tid(0) {}
	texture_array(unsigned width, unsigned height, unsigned layers, void * pixels) : texture_array(width, height, layers, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, pixels) {}
	texture_array(unsigned width, unsigned height, unsigned layers, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void * pixels);
	texture_array(texture_array && lhs);
	~texture_array();

	void bind(unsigned unit);
	void write(std::string const & fname, unsigned layer);

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	unsigned layers() const {return _l;}
	unsigned id() const {return _tid;}

	void operator=(texture_array && lhs);

	texture_array(texture_array &) = delete;
	void operator=(texture_array &) = delete;

private:
	unsigned _tid;
	unsigned _w, _h, _l;
};

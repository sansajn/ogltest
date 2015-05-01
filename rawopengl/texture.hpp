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

enum class texture_wrap  //! \sa glTexParameter
{
	clamp_to_edge,
	clamp_to_border,
	mirrored_repeat,
	repeat
};

enum class texture_filter  //! \sa glTexParameter
{
	nearest,
	linear,
	nearest_mipmap_nearest,
	linear_mipmap_nearest,
	nearest_mipmap_linear,
	linear_mipmap_linear
};

class texture  //!< \note not ment to be used directly \sa texture2d, ...
{
public:
	struct parameters
	{
		parameters();
		parameters & min(texture_filter mode);
		parameters & mag(texture_filter mode);
		parameters & wrap_s(texture_wrap mode);
		parameters & wrap_t(texture_wrap mode);
		parameters & wrap_r(texture_wrap mode);

		texture_filter min() const {return _min;}
		texture_filter mag() const {return _mag;}
		texture_wrap wrap_s() const {return _wrap[0];}
		texture_wrap wrap_t() const {return _wrap[1];}
		texture_wrap wrap_r() const {return _wrap[2];}

	private:
		texture_filter _min, _mag;
		texture_wrap _wrap[3];  // [s, t, r]
	};

	virtual ~texture();

	unsigned id() const {return _tid;}
	void bind(unsigned unit);

	texture(texture && lhs);
	void operator=(texture && lhs);

	texture(texture const &) = delete;
	void operator=(texture const &) = delete;

protected:
	texture() : _tid(0) {}
	texture(unsigned target, unsigned tid);
	texture(unsigned target, parameters const & params) : _tid(0), _target(target) {init(params);}

private:
	void init(parameters const & params);

	unsigned _tid;  //!< texture identifier \sa glGenTextures()
	unsigned _target;  //!< \sa glBindTexture()
};

class texture2d : public texture
{
public:
	texture2d();
	texture2d(std::string const & fname, parameters const & params = parameters());
	texture2d(unsigned width, unsigned height, sized_internal_format ifmt = sized_internal_format::rgba8, parameters const & params = parameters());
	texture2d(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, parameters const & params = parameters()) : texture2d(width, height, ifmt, pfmt, type, nullptr, params) {}
	texture2d(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void const * pixels, parameters const & params = parameters());
	texture2d(unsigned tid, unsigned width, unsigned height, pixel_format pfmt, pixel_type type);
	~texture2d();

	void bind_as_render_target(bool depth = true);
	void write(std::string const & fname);

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}

	texture2d(texture2d && lhs);
	void operator=(texture2d && lhs);

	texture2d(texture2d const &) = delete;
	void operator=(texture2d const &) = delete;

private:
	void create_framebuffer();
	void create_depthbuffer();
	void read(unsigned width, unsigned height, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void const * pixels);

	unsigned _fid;  //!< framebuffer identifier
	unsigned _rid;  //!< renderbuffer identifier
	unsigned _w, _h;
	pixel_format _fmt;
	pixel_type _type;  // TODO: nahrad sized_internal_format-om
};

class texture2d_array : public texture
{
public:
	texture2d_array() {}
	texture2d_array(unsigned width, unsigned height, unsigned layers, sized_internal_format ifmt, pixel_format pfmt, pixel_type type, void * pixels, parameters const & params = parameters());
	~texture2d_array() {}

	void write(std::string const & fname, unsigned layer);

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	unsigned layers() const {return _l;}

	texture2d_array(texture2d_array && lhs);
	void operator=(texture2d_array && lhs);

	texture2d_array(texture2d_array &) = delete;
	void operator=(texture2d_array &) = delete;

private:
	unsigned _w, _h, _l;
};

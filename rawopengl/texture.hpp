#pragma once
#include <string>

class texture
{
public:
	texture();
	texture(std::string const & fname);
	texture(unsigned width, unsigned height);
	texture(unsigned tid, unsigned width, unsigned height);
	~texture();

	void open(std::string const & fname);  // TODO: read() ?
	void create(unsigned width, unsigned height);
	void bind(unsigned unit);
	void bind_as_render_target(bool depth = true);

	unsigned id() const {return _tid;}
	unsigned width() const {return _w;}
	unsigned height() const {return _h;}

	texture(texture const &) = delete;
	void operator=(texture const &) = delete;

private:
	void create_framebuffer();
	void create_depthbuffer();

	unsigned _tid;  //!< texture identifier
	unsigned _fid;  //!< framebuffer identifier
	unsigned _rid;  //!< renderbuffer identifier
	unsigned _w, _h;
};

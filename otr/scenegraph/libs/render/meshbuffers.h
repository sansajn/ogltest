#pragma once

#include <memory>
#include <vector>
#include <GL/glew.h>
#include "render/buffer.h"

// ork/render

/*! Objekt scény je popísaný atributny napr. vrcholmi, farbou, normálami, ...
Trieda attribute_buffer popisuje jeden s týchto atributou. Spája dohromady dáta
(ako buffer) a popis dát. V podstate sa jedna o kontext pre funkciu
glVertexAttribPointer.
\saa glVertexAttribPointer() */
class attribute_buffer
{
public:
	// TODO: nahrad opengl enumy (absolutne matuce)
	typedef std::shared_ptr<gpubuffer> buffer_ptr;

	//! \saa glVertexAttribPointer()
	attribute_buffer(int index, int size, GLenum t, buffer_ptr b,
		int stride = 0, int offset = 0);

	attribute_buffer(int index, int size, GLenum t, bool norm, buffer_ptr b,
		int stride = 0, int offset = 0);

	int index() const {return _index;}
	int size() const {return _size;}  //!< number of components in attributes of this kind (i.e. pos:3 or 4, color:3, uv:2)
	GLenum type() const {return _type;}
	int attribute_size() const;  //!< sizeof attribute type
	buffer_ptr buf() {return _buf;}
	void buf(buffer_ptr b) {_buf = b;}
	int stride() const {return _stride;}
	int offset() const {return _offset;}
	bool norm() const {return _norm;}

private:
	int _index;
	int _size;
	GLenum _type;
	buffer_ptr _buf;
	int _stride;
	int _offset;
	bool _norm;
};

/*! združuje attributy objektu, draw metódy sú v podstate wrapper okolo
rodiny glDrawElements. Jedna sa o low-level strukturu. */
class mesh_buffers
{
public:
	typedef std::shared_ptr<attribute_buffer> attrbuf_ptr;

	GLenum mode;  //!< \saa #glDrawElements
	int nvertices;
	int nindices;

	mesh_buffers();
	virtual ~mesh_buffers();
	int attribute_count() const {return _attrs.size();}
	attrbuf_ptr attribute(int i) const {return _attrs[i];}
	attrbuf_ptr indices() const {return _indices;}
	void indices(attrbuf_ptr b) {_indices = b;}
	void append_attribute(int index, int size, int vertex_size, GLenum type, bool norm);
	void append_attribute(attrbuf_ptr b) {_attrs.push_back(b);}
	int primitive_count() const {return nindices > 0 ? nindices : nvertices;}
	void reset();  // TODO: zavadzajuci nazov

	void draw() const;  //!< \saa #glDrawElements

private:
	void set() const;
	void bind() const;
	void unbind() const;

	static GLenum _type;
	static mesh_buffers const * CURRENT;  //!< aby bol možný bind/unbind

	attrbuf_ptr _indices;
	std::vector<attrbuf_ptr> _attrs;
};

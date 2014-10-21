#pragma once
#include <memory>
#include <vector>
#include <GL/glew.h>
#include "core/ptr.h"
#include "buffer.h"
#include "types.h"

/*! Objekt scény je popísaný atributmi napr. vrcholmi, farbou, normálami, ...
Trieda attribute_buffer popisuje jeden s týchto atributou. Spája dohromady dáta
(ako buffer) a popis dát. V podstate sa jedna o kontext pre funkciu
glVertexAttribPointer.
\saa glVertexAttribPointer()
@ingroup render */
class attribute_buffer
{
public:
	//! \saa #glVertexAttribPointer
	attribute_buffer(int index, int size, attribute_type t, ptr<buffer> b,
		int stride = 0, int offset = 0);

	attribute_buffer(int index, int size, attribute_type t, bool norm, ptr<buffer> b,
		int stride = 0, int offset = 0);

	int index() const {return _index;}
	int size() const {return _size;}  //!< number of components in attributes of this kind (i.e. pos:3 or 4, color:3, uv:2)
	attribute_type type() const {return _type;}
	int attribute_size() const;  //!< sizeof all components in attributes of this kind
	ptr<buffer> buf() {return _buf;}
	void buf(ptr<buffer> b) {_buf = b;}  // TODO: toto naburava vnutornu integritu struktury
	int stride() const {return _stride;}
	int offset() const {return _offset;}
	bool norm() const {return _norm;}

private:
	int _index;
	int _size;
	attribute_type _type;
	ptr<buffer> _buf;
	int _stride;
	int _offset;
	bool _norm;
};  // attribute_buffer

/*! Združuje attributy objektu, draw metódy sú v podstate wrapper okolo
rodiny funkcii #glDrawElements. Jedna sa o low-level strukturu. */
class mesh_buffers
{
public:
	typedef std::shared_ptr<attribute_buffer> attrbuf_ptr;

	mesh_mode mode;  //!< default value TRIANGLES \saa #glDrawElements
	int nvertices;
	int nindices;

	mesh_buffers();  // TODO: pridaj konstruktor s mode parametrom
	virtual ~mesh_buffers();
	int attribute_count() const {return _attrs.size();}
	attrbuf_ptr attribute(int i) const {return _attrs[i];}
	attrbuf_ptr indices() const {return _indices;}
	void indices(attrbuf_ptr b) {_indices = b;}	
	void append_attribute(attrbuf_ptr b) {_attrs.push_back(b);}
	int primitive_count() const {return nindices > 0 ? nindices : nvertices;}
	void reset();  // TODO: zavadzajuci nazov

	void draw() const;  //!< \saa #glDrawElements

	void append_attribute(int index, int size, int vertex_size, attribute_type type, bool norm);  // TODO: toto zneprehladnuje kod, mala by ostat iba verzia s attribute_buffer

private:
	void set() const;
	void bind() const;
	void unbind() const;

	static attribute_type _type;
	static mesh_buffers const * CURRENT;  //!< aby bol možný bind/unbind

	attrbuf_ptr _indices;
	std::vector<attrbuf_ptr> _attrs;
};  // mesh_buffers

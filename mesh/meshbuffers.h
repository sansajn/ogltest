#pragma once

#include <memory>
#include <vector>
#include <GL/glew.h>
#include "buffer.h"

// ork/render

/*! Objekt scény je popísaný atributny napr. vrcholmi, farbou, normálami, ...
Trieda attribute_buffer popisuje jeden s týchto atributou. Spája dohromady dáta
(ako buffer) a popis dát. V podstate aa jedna o kontext pre funkciu
glVertexAttribPointer.
\saa glVertexAttribPointer() */
class attribute_buffer
{
public:
	typedef std::shared_ptr<gpubuffer> buffer_ptr;

	//! \saa glVertexAttribPointer()
	attribute_buffer(int index, int size, GLenum t, buffer_ptr b, int stride = 0,
		int offset = 0);

	int index() const {return _index;}
	int size() const {return _size;}
	GLenum type() const {return _type;}
	int attribute_size() const;
	buffer_ptr buf() {return _buf;}
	void buf(buffer_ptr b) {_buf = b;}
	int stride() const {return _stride;}
	int offset() const {return _offset;}

private:
	int _index;
	int _size;
	GLenum _type;
	buffer_ptr _buf;
	int _stride;
	int _offset;
};

/*! združuje attributy objektu, draw metódy sú v podstate wrapper okolo
rodiny glDrawElements */
class mesh_buffers
{
public:
	typedef std::shared_ptr<attribute_buffer> attrbuf_ptr;

	mesh_buffers() {}  // TODO: implement
	virtual ~mesh_buffers() {}  // TODO: implement
	int attribute_count() const {return _attrs.size();}
	attrbuf_ptr attribute(int i) const {return _attrs[i];}
	attrbuf_ptr indices() const {return _indices;}
	void indices(attrbuf_ptr b) {_indices = b;}
	void append_attribute(int index, int size, GLenum type);
	void append_attribute(int index, int size, int vertex_size, GLenum type, bool norm);
	void append_attribute(attrbuf_ptr b) {_attrs.push_back(b);}
	void reset();

	void draw(GLenum mode, int count);  //!< \saa glDrawElements()

private:
	void set();
	void bind();
	void unbind();

	// (?) tieto členy su v originalnej implementacii staticke, prečo ?
	GLenum _type;
	static mesh_buffers * CURRENT;  //!< aby bol možný bind/unbind

	attrbuf_ptr _indices;
	std::vector<attrbuf_ptr> _attrs;
};

#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include "meshbuffers.h"

template <typename Vertex, typename Index>
class mesh
{
public:
	typedef std::shared_ptr<mesh_buffers> meshbufs_ptr;

	mesh(GLenum mode);
	~mesh() {}
	int vertex_count() const {return _vertices.size();}
	int indice_count() const {return _indices.size();}
	void append_attribute_type(int id, int size, GLenum type, bool norm = false);
	void append_vertex(Vertex const & v) {_vertices.push_back(v);}
	void append_indice(Index i) {_indices.push_back(i);}
	meshbufs_ptr buf() const;
	GLenum mode() const {return _mode;}

private:
	void create_buffers() const;
	void upload_indices_to_gpu(GLenum u) const;
	void upload_vertices_to_gpu(GLenum u) const;

	std::vector<Index> _indices;
	std::vector<Vertex> _vertices;
	GLenum _mode;
	mutable meshbufs_ptr _buffers;
	mutable std::shared_ptr<gpubuffer> _index_buf;
	mutable std::shared_ptr<gpubuffer> _vertex_buf;
	mutable bool _created;
};

template <typename Vertex, typename Index>
mesh<Vertex, Index>::mesh(GLenum mode)
	: _mode(mode), _buffers(std::make_shared<mesh_buffers>()), _created(false)
{}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::append_attribute_type(int id, int size, GLenum type, bool norm)
{
	_buffers->append_attribute(id, size, sizeof(Vertex), type, norm);
}

template <typename Vertex, typename Index>
typename mesh<Vertex, Index>::meshbufs_ptr mesh<Vertex, Index>::buf() const
{
	if (!_created)
		create_buffers();
	return _buffers;
}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::create_buffers() const
{
	_vertex_buf = std::make_shared<gpubuffer>();
	upload_vertices_to_gpu(GL_STATIC_DRAW);

	// TODO: (?) preco sa vsade kopiruje jeden buffer ?
	assert(_buffers->attribute_count() > 0 && "no attributes");
	for (int i = 0; i < _buffers->attribute_count(); ++i)
		_buffers->attribute(i)->buf(_vertex_buf);

	if (_indices.size())
	{
		_index_buf = std::make_shared<gpubuffer>();
		upload_indices_to_gpu(GL_STATIC_DRAW);

		GLenum type;
		switch (sizeof(Index))
		{
			case 1:
				type = GL_UNSIGNED_BYTE;
				break;
			case 2:
				type = GL_UNSIGNED_SHORT;
				break;
			default:
				type = GL_UNSIGNED_INT;
				break;
		}

		_buffers->indices(
			std::make_shared<attribute_buffer>(-1, -1, type, _index_buf));
	}

	_created = true;
}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::upload_indices_to_gpu(GLenum u) const
{
	_index_buf->data(_indices.size() * sizeof(Index), &_indices[0], u);
}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::upload_vertices_to_gpu(GLenum u) const
{
	_vertex_buf->data(_vertices.size() * sizeof(Vertex), &_vertices[0], u);
}


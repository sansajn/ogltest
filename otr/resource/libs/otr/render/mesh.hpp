#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include "core/ptr.hpp"
#include "render/types.hpp"
#include "render/cast.hpp"
#include "render/meshbuffers.hpp"
#include "render/gpubuffer.hpp"

template <typename Vertex, typename Index = unsigned int>
class mesh
{
public:
	mesh(mesh_mode mode, mesh_usage usage);
	~mesh() {}
	int vertex_count() const {return _vertices.size();}
	int indice_count() const {return _indices.size();}
	void append_attribute_type(int id, int size, attribute_type t, bool norm = false);
	void append_vertex(Vertex const & v) {_vertices.push_back(v);}
	void append_indice(Index i) {_indices.push_back(i);}
	ptr<mesh_buffers> buf() const;
	mesh_mode mode() const {return _mode;}

private:
	void create_buffers() const;
	void upload_indices_to_gpu(buffer_usage u) const;
	void upload_vertices_to_gpu(buffer_usage u) const;

	std::vector<Index> _indices;
	std::vector<Vertex> _vertices;
	mesh_mode _mode;
	mesh_usage _usage;
	mutable ptr<mesh_buffers> _buffers;
	mutable std::shared_ptr<buffer> _index_buf;
	mutable std::shared_ptr<buffer> _vertex_buf;
	mutable bool _created;
	mutable bool _vertices_changed;
	mutable bool _indices_changed;
};

template <typename Vertex, typename Index>
mesh<Vertex, Index>::mesh(mesh_mode mode, mesh_usage usage)
	: _mode(mode), _usage(usage), _buffers(std::make_shared<mesh_buffers>()),
		_created(false), _vertices_changed(true), _indices_changed(true)
{}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::append_attribute_type(int id, int size, attribute_type t, bool norm)
{
	_buffers->append_attribute(id, size, sizeof(Vertex), t, norm);
}

template <typename Vertex, typename Index>
ptr<mesh_buffers> mesh<Vertex, Index>::buf() const
{
	if (!_created)
		create_buffers();

	if ((_usage == mesh_usage::GPU_DYNAMIC) || (_usage == mesh_usage::GPU_STREAM))
	{
		buffer_usage u = _usage == mesh_usage::GPU_DYNAMIC
			? buffer_usage::DYNAMIC_DRAW : buffer_usage::STREAM_DRAW;

		if (_vertices_changed)
			upload_vertices_to_gpu(u);

		if ((indice_count()) && _indices_changed)
			upload_indices_to_gpu(u);
	}

	return _buffers;
}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::create_buffers() const
{
	_buffers->mode = _mode;
	_vertex_buf = std::make_shared<gpubuffer>();
	upload_vertices_to_gpu(buffer_usage::STATIC_DRAW);
	_buffers->nvertices = _vertices.size();

	assert(_buffers->attribute_count() > 0 && "error: no attributes");
	for (int i = 0; i < _buffers->attribute_count(); ++i)
		_buffers->attribute(i)->buf(_vertex_buf);

	if (_indices.size())
	{
		_index_buf = std::make_shared<gpubuffer>();
		upload_indices_to_gpu(buffer_usage::STATIC_DRAW);

		attribute_type type;
		switch (sizeof(Index))
		{
			case 1:
				type = attribute_type::ui8;
				break;
			case 2:
				type = attribute_type::ui16;
				break;
			default:
				type = attribute_type::ui32;
				break;
		}

		_buffers->indices(
			std::make_shared<attribute_buffer>(-1, -1, type, _index_buf));

		_buffers->nindices = _indices.size();
	}

	_created = true;
}

// TODO: naozaj musi byt buffer iba implementacion gpubuffer ?

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::upload_indices_to_gpu(buffer_usage u) const
{
	ptr<gpubuffer> ib = std::dynamic_pointer_cast<gpubuffer>(_index_buf);
	if (ib)
		ib->data(_indices.size() * sizeof(Index), &_indices[0], u);
	else
		throw std::exception();  // TODO specify (indef-buffer musi byt implementaci gpubuffer)

	_indices_changed = false;
}

template <typename Vertex, typename Index>
void mesh<Vertex, Index>::upload_vertices_to_gpu(buffer_usage u) const
{
	ptr<gpubuffer> vb = std::dynamic_pointer_cast<gpubuffer>(_vertex_buf);
	if (vb)
		vb->data(_vertices.size() * sizeof(Vertex), &_vertices[0], u);
	else
		throw std::exception();  // TODO specify (_vertex_buf musi byt implementacia gpubuffer)

	_vertices_changed = false;
}

#include "model.hpp"

using std::string;
using std::shared_ptr;

namespace gl {

void model::render() const
{
	// TODO: tu potrebujem pre kazdy mesh bindnut textury
	for (shared_ptr<mesh> m : _meshes)
		m->render();
}

void model::append_mesh(shared_ptr<mesh> m)
{
	_meshes.push_back(m);
}

void model::append_mesh(shared_ptr<mesh> m, string const & texture_id)
{
	_meshes.push_back(m);
	_texture_ids.push_back(texture_id);
}

model::model(model && other)
	: _texture_ids{move(other._texture_ids)}, _meshes{move(other._meshes)}
{}

void model::operator=(model && other)
{
	swap(_meshes, other._meshes);
	swap(_texture_ids, other._texture_ids);
}

}  // gl

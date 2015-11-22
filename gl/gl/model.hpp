#pragma once
#include <vector>
#include <string>
#include <memory>
#include "mesh.hpp"

namespace gl {

class model
{
public:
	model() {}
	model(model && other);
	virtual ~model() {}
	virtual void render() const;
	void append_mesh(std::shared_ptr<mesh> m);
	void append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id);
	void operator=(model && other);

	model(model const &) = delete;
	void operator=(model const &) = delete;

private:
	std::vector<std::string> _texture_ids;
	std::vector<std::shared_ptr<mesh>> _meshes;
};

}  // gl

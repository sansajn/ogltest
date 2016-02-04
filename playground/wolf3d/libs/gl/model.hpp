#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/texture.hpp"


namespace gl {

struct property  // TODO: model-property/mesh-property
{
	virtual ~property() {}
	virtual void bind(shader::program & p) = 0;
};

class model
{
public:
	model() {}
	model(model && other);
	virtual ~model();
	void append_mesh(std::shared_ptr<mesh> m);
	void append_mesh(std::shared_ptr<mesh> m, std::vector<property *> const & mesh_props);
	void append_global(property * prop);
	virtual void render(shader::program & p) const;
	void operator=(model && other);

	model(model const &) = delete;
	void operator=(model const &) = delete;

private:
	using propvect = std::vector<property *>;  // predpoklada alokaciu pomocou new

	std::vector<std::shared_ptr<mesh>> _meshes;
	std::vector<propvect> _props;  //!< vlastnosti pre kazdu mriezku
	propvect _glob_props;  //!< spolocne vlastnosti pre vsetky mriezky
};


struct texture_property : public property
{
	texture_property(std::shared_ptr<texture2d> tex, std::string const & uname, unsigned bind_unit = 0);
	void bind(shader::program & p) override;

	std::shared_ptr<texture2d> _tex;
	std::string _uname;
	unsigned _bind_unit;
};

//! material povrchu pre phongov osvetlovaci model
struct material_property : public property
{
	material_property(glm::vec3 const & ambient = glm::vec3{.2}, float intensity = 1.0f, float shininess = 64.0f);
	void bind(shader::program & p) override;

	glm::vec3 ambient;
	float intensity;
	float shininess;
};

}  // gl

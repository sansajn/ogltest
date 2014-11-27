#pragma once
#include <string>
#include "core/ptr.hpp"
#include "render/meshbuffers.hpp"
#include "resource/mesh_loader.hpp"

/*! @ingroup resource */
class orkmesh_loader : public mesh_loader
{
public:
	ptr<mesh_buffers> load(std::string const & fname) override;
	bool load(std::string const & fname, mesh_buffers * m) override;
};

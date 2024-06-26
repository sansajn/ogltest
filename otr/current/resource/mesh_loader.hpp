#pragma once
#include <string>
#include "core/ptr.hpp"
#include "render/meshbuffers.hpp"

/*! @ingroup resource */
class mesh_loader
{
public:
	virtual ptr<mesh_buffers> load(std::string const & fname) = 0;
	virtual bool load(std::string const & fname, mesh_buffers * m) = 0;
};

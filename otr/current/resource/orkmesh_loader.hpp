#pragma once
#include <string>
#include "core/ptr.hpp"
#include "render/meshbuffers.h"
#include "mesh_loader.hpp"

/*! @ingroup resource */
class orkmesh_loader : public mesh_loader
{
public:
	ptr<mesh_buffers> load(std::string const & fname);
};

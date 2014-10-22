#pragma once
#include <string>
#include "core/ptr.h"
#include "render/meshbuffers.h"
#include "mesh_loader.h"

/*! @ingroup resource */
class orkmesh_loader : public mesh_loader
{
public:
	ptr<mesh_buffers> load(std::string const & fname);
};

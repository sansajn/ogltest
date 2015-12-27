#pragma once
#include <string>
#include "core/ptr.h"
#include "render/meshbuffers.h"

/*! @ingroup resource */
class mesh_loader
{
public:
	virtual ptr<mesh_buffers> load(std::string const & fname) = 0;
};

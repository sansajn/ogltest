// umoznuje citanie mesh objektou roznych formatou (pomocou kniznice assimp)
#pragma once
#include "mesh_loader.h"

/*! \ingroup resource */
class assimp_loader : public mesh_loader
{
public:
	ptr<mesh_buffers> load(std::string const & fname) override;
};

#pragma once
#include <memory>
#include <boost/noncopyable.hpp>
#include "gles2/mesh_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/program_gles2.hpp"
#include "game.hpp"

class medkit_object : private boost::noncopyable
{
public:
	medkit_object(btVector3 const & position);
	~medkit_object();
	void update(float dt);
	void render(gles2::shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(phys::rigid_body_world & world, int mark = -1);
	btCollisionObject * collision() const {return _collision.native();}

private:
	std::shared_ptr<btCollisionShape> shared_shape();

	phys::trigger_object _collision;
	glm::quat _rot;
	gles2::mesh _mesh;  // TODO: zdielaj
	gles2::texture2d _tex;  // TODO: zdielaj
	static std::shared_ptr<btCollisionShape> _shape;
};

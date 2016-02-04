#pragma once
#include <memory>
#include <boost/noncopyable.hpp>
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "game.hpp"

class medkit_object : private boost::noncopyable
{
public:
	medkit_object(btVector3 const & position);
	~medkit_object();
	void update(float dt);
	void render(shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(phys::rigid_body_world & world, int mark = -1);
	btCollisionObject * collision() const {return _collision.native();}

private:
	std::shared_ptr<btCollisionShape> shared_shape();

	phys::trigger_object _collision;
	glm::quat _rot;
	gl::mesh _mesh;  // TODO: zdielaj
	texture2d _tex;  // TODO: zdielaj
	static std::shared_ptr<btCollisionShape> _shape;
};

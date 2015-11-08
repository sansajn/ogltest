#pragma once
#include "game.hpp"
#include "gl/program.hpp"
#include "gl/mesh.hpp"
#include "gl/texture.hpp"

class enemy_object
	: public game_object, private boost::noncopyable
{
public:
	enemy_object(btVector3 const & position, gl::mesh * m, texture2d * t);
	~enemy_object();
	btTransform const & transform() const;

	void update(float dt);
	void render(shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(phys::rigid_body_world & world, int mask = -1);  // const

private:
	std::shared_ptr<btCollisionShape> shared_shape();

	phys::body_object _collision;
	glm::quat _rot;
	gl::mesh * _mesh;
	texture2d * _diff_tex;  // TODO: model

	static std::shared_ptr<btCollisionShape> _shape;
};

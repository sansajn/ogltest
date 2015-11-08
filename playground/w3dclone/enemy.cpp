#include "enemy.hpp"

using std::shared_ptr;
using glm::vec3;
using glm::mat4;

using namespace phys;

shared_ptr<btCollisionShape> enemy_object::_shape;

enemy_object::enemy_object(btVector3 const & position, gl::mesh * m, texture2d * t)
	: _mesh{m}, _diff_tex{t}
{
	assert(_mesh && _diff_tex && "invalid ponter");
	_collision = body_object{shared_shape(), 90.0f, position + btVector3{.5, .5, -.5}};
}

enemy_object::~enemy_object()
{
	if (_shape.use_count() == 1)
		_shape.reset();
}

btTransform const & enemy_object::transform() const
{
	return _collision.transform();
}

void enemy_object::update(float dt)
{
	auto const & play = game_world::ref().player()->transform();
	_rot = glm_cast(-play.getRotation());
}

void enemy_object::render(shader::program & p, glm::mat4 const & world_to_screen)
{
	mat4 T = translate(glm_cast(_collision.position()));
	mat4 R = mat4_cast(_rot);
	mat4 M = T * R * scale(.4f * vec3{28.0/55.0, 1, 1});
	p.uniform_variable("local_to_screen", world_to_screen * M);
	_diff_tex->bind(0);
	p.uniform_variable("s", 0);
	_mesh->render();
}

void enemy_object::link_with(rigid_body_world & world, int mask)
{
	world.native()->addRigidBody(_collision.native());
	if (mask != -1)
		_collision.native()->setUserIndex(mask);
}

std::shared_ptr<btCollisionShape> enemy_object::shared_shape()
{
	if (!_shape)
		_shape = make_box_shape(btVector3{.25, .4, .25});
	return _shape;
}

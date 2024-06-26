#include "medkit.hpp"
#include "gl/texture_loader.hpp"
#include "gl/shapes.hpp"

using std::string;
using glm::vec3;
using glm::mat4;
using glm::translate;
using glm::scale;
using gl::mesh;
using gl::texture_from_file;

using namespace phys;

char const * medkit_texture_path = "assets/textures/medkit.png";

std::shared_ptr<btCollisionShape> medkit_object::_shape;

medkit_object::medkit_object(btVector3 const & position)
	: _collision{shared_shape(), position + btVector3{.5, .15, -.5}}  // must levitate over the ground to avoid collision
{
	_mesh = gl::make_quad_xy<gl::mesh>();
	_tex = texture_from_file(medkit_texture_path);
}

medkit_object::~medkit_object()
{
	if (_shape.use_count() == 1)
		_shape.reset();
}

void medkit_object::update(float dt)
{
	auto const & play = game_world::ref().player()->transform();
	_rot = glm_cast(-play.getRotation());
}

void medkit_object::render(shader::program & p, glm::mat4 const & world_to_screen)
{
	mat4 T = translate(glm_cast(_collision.position()));
	mat4 R = mat4_cast(_rot);
	mat4 M = T * R * scale(0.1f * vec3{1, 19.0f/28.0f, 1});
	p.uniform_variable("local_to_screen", world_to_screen*M);
	_tex.bind(0);
	p.uniform_variable("s", 0);
	_mesh.render();
}

void medkit_object::link_with(rigid_body_world & world, int mark)
{
	world.link(_collision);
	if (mark != -1)
		_collision.native()->setUserIndex(mark);
}

std::shared_ptr<btCollisionShape> medkit_object::shared_shape()
{
	if (!_shape)
		_shape = make_box_shape(btVector3{.2, .1, .2});
	return _shape;
}

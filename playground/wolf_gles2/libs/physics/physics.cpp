#include "physics.hpp"
#include <algorithm>
#include "physics/debug_draw_gles2.hpp"

using std::swap;
using std::make_pair;
using std::set_difference;
using std::shared_ptr;

namespace phys {

static_body_object::static_body_object() : _body{nullptr}
{}

static_body_object::static_body_object(shared_ptr<btCollisionShape> shape, btVector3 const & position, btQuaternion const & rotation)
	: _shape{shape}
{
	_body = new btRigidBody{
		btRigidBody::btRigidBodyConstructionInfo{0, nullptr, _shape.get()}};
	_body->setWorldTransform(btTransform{rotation, position});
}

static_body_object::static_body_object(static_body_object && other)
	: _body{other._body}, _shape{move(other._shape)}
{
	other._body = nullptr;
}

void static_body_object::operator=(static_body_object && other)
{
	std::swap(_body, other._body);
	std::swap(_shape, other._shape);
}

static_body_object::~static_body_object()
{
	delete _body;
}

btVector3 const & static_body_object::position() const
{
	return _body->getWorldTransform().getOrigin();
}

btQuaternion static_body_object::rotation() const
{
	return _body->getWorldTransform().getRotation();
}

btTransform const & static_body_object::transform() const
{
	return _body->getWorldTransform();
}

void static_body_object::transform(btTransform const & t)
{
	_body->setWorldTransform(t);
}

btRigidBody * static_body_object::native() const
{
	return _body;
}


body_object::body_object() : _body{nullptr}
{}

body_object::body_object(shared_ptr<btCollisionShape> shape, float mass, btVector3 const & position,
	btQuaternion const & rotation) : _shape{shape}
{
	btVector3 local_inertia{0,0,0};
	if (mass != 0)
		_shape->calculateLocalInertia(mass, local_inertia);

	_body = new btRigidBody{
		btRigidBody::btRigidBodyConstructionInfo{mass, nullptr, _shape.get(), local_inertia}};
	_body->setWorldTransform(btTransform(rotation, position));
}

body_object::body_object(body_object && other)
	: _body{other._body}, _shape{move(other._shape)}
{
	other._body = nullptr;
}

void body_object::operator=(body_object && other)
{
	std::swap(_body, other._body);
	std::swap(_shape, other._shape);
}

body_object::~body_object()
{
	delete _body;
}

btVector3 const & body_object::position() const
{
	return _body->getWorldTransform().getOrigin();
}

btQuaternion body_object::rotation() const
{
	return _body->getWorldTransform().getRotation();
}

btTransform const & body_object::transform() const
{
	return _body->getWorldTransform();
}

void body_object::transform(btTransform const & t)
{
	_body->setWorldTransform(t);
}

btRigidBody * body_object::native() const
{
	return _body;
}


trigger_object::trigger_object() : _collision{nullptr}
{}

trigger_object::trigger_object(shared_ptr<btCollisionShape> shape, btVector3 const & position, btQuaternion const & rotation)
	: _shape{shape}
{
	_collision = new btCollisionObject{};
	_collision->setCollisionShape(_shape.get());
	_collision->setWorldTransform(btTransform{rotation, position});
	_collision->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

trigger_object::trigger_object(trigger_object && other)
	: _collision{other._collision}, _shape{other._shape}
{
	other._collision = nullptr;
}

void trigger_object::operator=(trigger_object && other)
{
	std::swap(_collision, other._collision);
	std::swap(_shape, other._shape);
}

trigger_object::~trigger_object()
{
	delete _collision;
}

btVector3 const & trigger_object::position() const
{
	return _collision->getWorldTransform().getOrigin();
}

btQuaternion trigger_object::rotation() const
{
	return _collision->getWorldTransform().getRotation();
}

btTransform const & trigger_object::transform() const
{
	return _collision->getWorldTransform();
}

void trigger_object::transform(btTransform const & t)
{
	_collision->setWorldTransform(t);
}

btCollisionObject * trigger_object::native() const
{
	return _collision;
}


rigid_body_world::rigid_body_world()
{
	_collision_configuration = new btDefaultCollisionConfiguration{};
	_dispatcher = new btCollisionDispatcher{_collision_configuration};
	_broadhpase = new btDbvtBroadphase{};
	_solver = new btSequentialImpulseConstraintSolver{};
	_world = new btDiscreteDynamicsWorld{_dispatcher, _broadhpase, _solver, _collision_configuration};
}

rigid_body_world::~rigid_body_world()
{
	delete _world;
	delete _solver;
	delete _broadhpase;
	delete _dispatcher;
	delete _collision_configuration;

	if (_build_in_debug_used)
		delete _debug;
}

void rigid_body_world::update(float dt)
{
	_world->stepSimulation(dt);
	handle_collisions();
}

void rigid_body_world::link(static_body_object const & o)
{
	_world->addRigidBody(o.native());
}

void rigid_body_world::link(body_object const & o)
{
	_world->addRigidBody(o.native());
}

void rigid_body_world::link(trigger_object const & o)
{
	_world->addCollisionObject(o.native());
}

void rigid_body_world::unlink(static_body_object const & o)
{
	_world->removeRigidBody(o.native());
}

void rigid_body_world::unlink(body_object const & o)
{
	_world->removeRigidBody(o.native());
}

void rigid_body_world::unlink(trigger_object const & o)
{
	_world->removeCollisionObject(o.native());
}

void rigid_body_world::debug_render(glm::mat4 const & world_to_screen)
{
	if (!_debug)
	{
		auto * debug = new debug_draw_impl{};
		debug->toggle_debug_flag(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
		debug_drawer(debug);
		_build_in_debug_used = true;
	}

	if (_build_in_debug_used)
		static_cast<debug_draw_impl *>(_debug)->update(world_to_screen);

	if (_debug)
		_world->debugDrawWorld();
}

void rigid_body_world::debug_drawer(btIDebugDraw * draw)
{
	if (_build_in_debug_used)
	{
		delete _debug;
		_build_in_debug_used = false;
	}

	_debug = draw;
	_world->setDebugDrawer(_debug);
}

void rigid_body_world::add_collision_listener(collision_listener * l)
{
	_listeners.push_back(l);
}

void rigid_body_world::remove_collision_listener(collision_listener * l)
{
	auto it = std::find(_listeners.begin(), _listeners.end(), l);
	if (it != _listeners.end())
		_listeners.erase(it);
}

void rigid_body_world::handle_collisions()
{
	// collisions this update
	collision_pairs pairs_this_update;
	for (int i = 0; i < _dispatcher->getNumManifolds(); ++i)
	{
		btPersistentManifold * manifold = _dispatcher->getManifoldByIndexInternal(i);
		if (manifold->getNumContacts() > 0)
		{
			auto body0 = manifold->getBody0();
			auto body1 = manifold->getBody1();

			// always create the pair in a predictable order (v com je tato metoda predictable ?)
			bool const swapped = body0 > body1;
			auto sorted_body_a = swapped ? body1 : body0;
			auto sorted_body_b = swapped ? body0 : body1;
			auto collision = std::make_pair(sorted_body_a, sorted_body_b);
			pairs_this_update.insert(collision);

			if (_last_update_collisions.find(collision) == _last_update_collisions.end())
				collision_event((btCollisionObject *)body0, (btCollisionObject *)body1);
		}
	}

	// collisions removed this update
	collision_pairs removed_pairs;
	set_difference(_last_update_collisions.begin(), _last_update_collisions.end(),
		pairs_this_update.begin(), pairs_this_update.end(), inserter(removed_pairs, removed_pairs.begin()));

	for (auto const & collision : removed_pairs)
		separation_event((btCollisionObject *)collision.first, (btCollisionObject *)collision.second);

	std::swap(_last_update_collisions, pairs_this_update);
}

void rigid_body_world::collision_event(btCollisionObject * body0, btCollisionObject * body1)
{
	for (auto * l : _listeners)
		l->collision_event(body0, body1);
}

void rigid_body_world::separation_event(btCollisionObject * body0, btCollisionObject * body1)
{
	for (auto * l : _listeners)
		l->separation_event(body0, body1);
}

}  // phys

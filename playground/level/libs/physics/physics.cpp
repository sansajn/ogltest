#include "physics.hpp"
#include <algorithm>

using std::swap;
using std::make_pair;
using std::set_difference;

physics_object::physics_object() : _body{nullptr}
{}

physics_object::physics_object(std::shared_ptr<btCollisionShape> shape, float mass, btVector3 const & position,
	btQuaternion const & rotation) : _motion{btTransform{rotation, position}}, _shape{shape}
{
	btVector3 local_inertia{0,0,0};
	if (mass != 0)
		_shape->calculateLocalInertia(mass, local_inertia);

	_body = new btRigidBody{
		btRigidBody::btRigidBodyConstructionInfo{mass, &_motion, _shape.get(), local_inertia}};
}

physics_object::physics_object(physics_object && other)
	: _body{other._body}, _motion{other._motion}, _shape{move(other._shape)}
{
	other._body = nullptr;
	if (_body)
		_body->setMotionState(&_motion);
}

void physics_object::operator=(physics_object && other)
{
	swap(_body, other._body);
	swap(_motion, other._motion);
	swap(_shape, other._shape);

	if (_body)
		_body->setMotionState(&_motion);

	if (other._body)
		other._body->setMotionState(&other._motion);
}

physics_object::~physics_object()
{
	delete _body;
}

btVector3 const & physics_object::position() const
{
	return _body->getCenterOfMassPosition();
}

btQuaternion physics_object::rotation() const
{
	return _body->getOrientation();
}

physics_trigger::physics_trigger(btVector3 const & box_half_extends, btVector3 const & position, btQuaternion const & rotation)
	: _shape{box_half_extends}
{
	_trigger.setCollisionShape(&_shape);
	_trigger.setWorldTransform(btTransform{rotation, position});
	_trigger.setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

physics_trigger::physics_trigger(physics_trigger && other)
	: _shape{other._shape}, _trigger{other._trigger}
{
	_trigger.setCollisionShape(&_shape);
}

void physics_trigger::operator=(physics_trigger && other)
{
	_shape = other._shape;
	_trigger = other._trigger;
	_trigger.setCollisionShape(&_shape);
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
}

void rigid_body_world::update(float dt)
{
	_world->stepSimulation(dt);
	check_for_collision_event();
}

void rigid_body_world::add_collision_listener(collision_listener * l)
{
	_listeners.push_back(l);
}

void rigid_body_world::collision_event(btRigidBody * body0, btRigidBody * body1)
{
	for (auto l : _listeners)
		l->collision_event(body0, body1);
}

void rigid_body_world::separation_event(btRigidBody * body0, btRigidBody * body1)
{
	for (auto l : _listeners)
		l->separation_event(body0, body1);
}

void rigid_body_world::check_for_collision_event()
{
	// collisions this update
	collision_pairs pairs_this_update;
	for (int i = 0; i < _dispatcher->getNumManifolds(); ++i)
	{
		btPersistentManifold * manifold = _dispatcher->getManifoldByIndexInternal(i);
		if (manifold->getNumContacts() > 0)
		{
			btRigidBody const * body0 = static_cast<btRigidBody const *>(manifold->getBody0());  // TODO: nemalo by to byt dynamic_cast ?
			btRigidBody const * body1 = static_cast<btRigidBody const *>(manifold->getBody1());

			// always create the pair in a predictable order (v com je tato metoda predictable ?)
			bool const swapped = body0 > body1;
			btRigidBody const * sorted_body_a = swapped ? body1 : body0;
			btRigidBody const * sorted_body_b = swapped ? body0 : body1;
			auto collision = make_pair(sorted_body_a, sorted_body_b);
			pairs_this_update.insert(collision);

			if (_last_update_collisions.find(collision) == _last_update_collisions.end())
				collision_event((btRigidBody *)body0, (btRigidBody *)body1);
		}
	}

	// collisions removed this update
	collision_pairs removed_pairs;
	set_difference(_last_update_collisions.begin(), _last_update_collisions.end(),
		pairs_this_update.begin(), pairs_this_update.end(), inserter(removed_pairs, removed_pairs.begin()));

	for (auto const & collision : removed_pairs)
		separation_event((btRigidBody *)collision.first, (btRigidBody *)collision.second);

	swap(_last_update_collisions, pairs_this_update);
}

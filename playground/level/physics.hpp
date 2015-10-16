#pragma once
#include <memory>
#include <set>
#include <bullet/btBulletDynamicsCommon.h>

class physics_object
{
public:
	physics_object();

	physics_object(std::shared_ptr<btCollisionShape> shape, float mass = 0,
		btVector3 const & position = btVector3{0,0,0}, btQuaternion const & rotation = btQuaternion{0,0,1,1});

	physics_object(physics_object && other);
	~physics_object();
	btRigidBody * body() const {return _body;}
	void operator=(physics_object && other);

	physics_object(physics_object const &) = delete;
	physics_object & operator=(physics_object const &) = delete;

private:
	btRigidBody * _body;
	btDefaultMotionState _motion;
	std::shared_ptr<btCollisionShape> _shape;
};

class rigid_body_world  //!< svet tuhych telies a ich interakcii
{
public:
	rigid_body_world();
	~rigid_body_world();

	void add(btRigidBody * body) {_world->addRigidBody(body);}
	void simulate(float dt);
	void debug_draw() {_world->debugDrawWorld();}
	void debug_drawer(btIDebugDraw * ddraw) {_world->setDebugDrawer(ddraw);}

	// world events
	void collision_event(btRigidBody * body0, btRigidBody * body1);
	void separation_event(btRigidBody * body0, btRigidBody * body1);

	// cooperation
	btDynamicsWorld * world() const {return _world;}

	void operator=(rigid_body_world const &) = delete;
	rigid_body_world(rigid_body_world const &) = delete;

private:
	void check_for_collision_event();

	using collision_pairs = std::set<std::pair<btRigidBody const *, btRigidBody const *>>;
	collision_pairs _last_update_collisions;

	btDynamicsWorld * _world;
	btBroadphaseInterface * _broadhpase;
	btCollisionConfiguration * _collision_configuration;
	btCollisionDispatcher * _dispatcher;
	btConstraintSolver * _solver;
};

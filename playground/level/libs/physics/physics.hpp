#pragma once
#include <memory>
#include <vector>
#include <set>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>

// TODO: phys namespace ?

class physics_object  // TODO: phys::body_object
{
public:
	physics_object();

	physics_object(std::shared_ptr<btCollisionShape> shape, float mass = 0,
		btVector3 const & position = btVector3{0,0,0}, btQuaternion const & rotation = btQuaternion{0,0,0,1});

	physics_object(physics_object && other);
	~physics_object();
	btVector3 const & position() const;
	btQuaternion rotation() const;
	btRigidBody * body() const {return _body;}
	void operator=(physics_object && other);

	physics_object(physics_object const &) = delete;
	physics_object & operator=(physics_object const &) = delete;

private:
	btRigidBody * _body;
	btDefaultMotionState _motion;
	std::shared_ptr<btCollisionShape> _shape;
};

class physics_trigger  // TODO: phys::trigger_object
{
public:
	physics_trigger() : _shape{btVector3{0,0,0}} {}
	physics_trigger(btVector3 const & box_half_extends, btVector3 const & position = btVector3{0,0,0}, btQuaternion const & rotation = btQuaternion::getIdentity());
	physics_trigger(physics_trigger && other);
	btCollisionObject * collision() const {return const_cast<btCollisionObject *>(&_trigger);}
	void operator=(physics_trigger && other);

	physics_trigger(physics_trigger const &) = delete;
	physics_trigger & operator=(physics_trigger const &) = delete;

private:
	btBoxShape _shape;
	btCollisionObject _trigger;
};

struct collision_listener
{
	virtual ~collision_listener() {}
	virtual void collision_event(btRigidBody * body0, btRigidBody * body1) {}
	virtual void separation_event(btRigidBody * body0, btRigidBody * body1) {}
};

class rigid_body_world  //!< svet tuhych telies a ich interakcii
{
public:
	rigid_body_world();
	virtual ~rigid_body_world();

	void add(btRigidBody * body) {_world->addRigidBody(body);}  // link/unlink, connect/unconnect
	void add_collision(btCollisionObject * collision) {_world->addCollisionObject(collision);}
	void update(float dt);
	void debug_draw() {_world->debugDrawWorld();}  // TODO: debug_render()
	void debug_drawer(btIDebugDraw * ddraw) {_world->setDebugDrawer(ddraw);}
	void add_collision_listener(collision_listener * l);

	// cooperation
	btDynamicsWorld * world() const {return _world;}

	void operator=(rigid_body_world const &) = delete;
	rigid_body_world(rigid_body_world const &) = delete;

private:
	void check_for_collision_event();
	void collision_event(btRigidBody * body0, btRigidBody * body1);
	void separation_event(btRigidBody * body0, btRigidBody * body1);

	using collision_pairs = std::set<std::pair<btRigidBody const *, btRigidBody const *>>;
	collision_pairs _last_update_collisions;

	btDynamicsWorld * _world;
	btBroadphaseInterface * _broadhpase;
	btCollisionConfiguration * _collision_configuration;
	btCollisionDispatcher * _dispatcher;
	btConstraintSolver * _solver;

	std::vector<collision_listener *> _listeners;
};

inline std::shared_ptr<btCollisionShape> make_box_shape(btVector3 const & half_extents)
{
	return std::shared_ptr<btCollisionShape>{new btBoxShape{half_extents}};
}

inline std::shared_ptr<btCollisionShape> make_sphere_shape(float r)
{
	return std::shared_ptr<btCollisionShape>{new btSphereShape{r}};
}

inline std::shared_ptr<btCollisionShape> make_cylinder_shape(float r, float h)
{
	return std::shared_ptr<btCollisionShape>{new btCylinderShape{btVector3{r, h/2, 0}}};
}

inline btVector3 bullet_cast(glm::vec3 const & v)
{
	return btVector3{v.x, v.y, v.z};
}

inline glm::vec3 glm_cast(btVector3 const & v)
{
	return glm::vec3{v[0], v[1], v[2]};
}

inline glm::quat glm_cast(btQuaternion const & q)
{
	return glm::quat{q.w(), q.x(), q.y(), q.z()};
}

inline glm::mat3 glm_cast(btMatrix3x3 const & m)
{
	btVector3 const & r0 = m.getRow(0);
	btVector3 const & r1 = m.getRow(1);
	btVector3 const & r2 = m.getRow(2);
	return glm::mat3{
		r0.x(), r0.y(), r0.z(),
		r1.x(), r1.y(), r1.z(),
		r2.x(), r2.y(), r2.z()};
}

inline glm::mat4 glm_cast(btTransform const & t)
{
	// TODO: vyslednu maticu je mozne zostavit 'manualne' tzn. bez vypoctu
	return glm::translate(glm_cast(t.getOrigin())) * glm::mat4{glm_cast(t.getBasis())};
}

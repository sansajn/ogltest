#pragma once
#include <memory>
#include <vector>
#include <set>
#include <boost/noncopyable.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>


namespace phys {

//! nehybne tuhe teleso
class static_body_object : private boost::noncopyable
{
public:
	static_body_object();  //!< makes uninitialized object

	static_body_object(std::shared_ptr<btCollisionShape> shape, btVector3 const & position = btVector3{0,0,0},
		btQuaternion const & rotation = btQuaternion{0,0,0,1});

	static_body_object(static_body_object && other);
	~static_body_object();

	btVector3 const & position() const;
	btQuaternion rotation() const;  // TODO: neefektivne
	btTransform const & transform() const;
	void transform(btTransform const & t);
	btRigidBody * native() const;
	void operator=(static_body_object && other);

private:
	btRigidBody * _body;
	std::shared_ptr<btCollisionShape> _shape;
};

//! tuhe teleso
class body_object : private boost::noncopyable
{
public:
	body_object();  //!< makes uninitialized object

	body_object(std::shared_ptr<btCollisionShape> shape, float mass = 0, btVector3 const & position = btVector3{0,0,0},
		btQuaternion const & rotation = btQuaternion{0,0,0,1});

	body_object(body_object && other);
	~body_object();

	btVector3 const & position() const;
	btQuaternion rotation() const;  // TODO: neefektivne
	btTransform const & transform() const;
	void transform(btTransform const & t);
	btRigidBody * native() const;
	void operator=(body_object && other);

private:
	btRigidBody * _body;
	std::shared_ptr<btCollisionShape> _shape;
};

//! oblast reagujuca na koliziu, nepodliehajuca smulacii
class trigger_object : private boost::noncopyable
{
public:
	trigger_object();  //!< vytvori neinicializovany objekt

	trigger_object(std::shared_ptr<btCollisionShape> shape, btVector3 const & position = btVector3{0,0,0},
		btQuaternion const & rotation = btQuaternion{0,0,0,1});

	trigger_object(trigger_object && other);
	~trigger_object();

	btVector3 const & position() const;
	btQuaternion rotation() const;  // TODO: neefektivne
	btTransform const & transform() const;
	void transform(btTransform const & t);
	btCollisionObject * native() const;
	void operator=(trigger_object && other);

private:
	btCollisionObject * _collision;
	std::shared_ptr<btCollisionShape> _shape;
};


struct collision_listener
{
	virtual ~collision_listener() {}
	virtual void collision_event(btCollisionObject * body0, btCollisionObject * body1) {}
	virtual void separation_event(btCollisionObject * body0, btCollisionObject * body1) {}
};

//! svet tuhych telies
class rigid_body_world : private boost::noncopyable
{
public:
	rigid_body_world();
	virtual ~rigid_body_world();
	void update(float dt);
	void link(static_body_object const & o);
	void link(body_object const & o);
	void link(trigger_object const & o);
	void unlink(static_body_object const & o);
	void unlink(body_object const & o);
	void unlink(trigger_object const & o);
	void debug_render(glm::mat4 const & world_to_screen);
	void debug_drawer(btIDebugDraw * ddraw);
	void add_collision_listener(collision_listener * l);
	void remove_collision_listener(collision_listener * l);
	btDynamicsWorld * native() const {return _world;}

private:
	void handle_collisions();
	void collision_event(btCollisionObject * body0, btCollisionObject * body1);
	void separation_event(btCollisionObject * body0, btCollisionObject * body1);

	using collision_pairs = std::set<std::pair<btCollisionObject const *, btCollisionObject const *>>;
	collision_pairs _last_update_collisions;

	btDynamicsWorld * _world;
	btBroadphaseInterface * _broadhpase;
	btCollisionConfiguration * _collision_configuration;
	btCollisionDispatcher * _dispatcher;
	btConstraintSolver * _solver;
	btIDebugDraw * _debug = nullptr;
	bool _build_in_debug_used = false;
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

}  // phys

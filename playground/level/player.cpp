#include "player.hpp"
#include "geometry/utility.hpp"

using glm::ivec2;
using glm::vec3;
using glm::mat3;
using glm::quat;
using glm::mat3_cast;
using glm::radians;
using gl::camera;
using geom::right;
using geom::forward;
using ui::glut_pool_window;

void fps_move::init(glut_pool_window::user_input * in, btRigidBody * body, float velocity)
{
	assert(in && body && "invalid pointer");
	_in = in;
	_body = body;
	_velocity = velocity;
	strcpy(_controls, "wsad");  // default controls
}

void fps_move::input(float dt)
{
	vec3 vel{0,0,0};

	if (_in->one_of_key(_controls))
	{
		mat3 r = mat3_cast(glm_cast(_body->getOrientation()));
		vec3 right_dir = right(r);
		vec3 forward_dir = forward(r);

		if (_in->key(_controls[(int)key::forward]))
			vel -= forward_dir;
		if (_in->key(_controls[(int)key::backward]))
			vel += forward_dir;
		if (_in->key(_controls[(int)key::left]))
			vel -= right_dir;
		if (_in->key(_controls[(int)key::right]))
			vel += right_dir;

		vel.y = 0;  // chcem sa pohybovat iba po rovine
		vel *= _velocity;
	}

	_body->setLinearVelocity(bullet_cast(vel));
}

void fps_look::init(glut_pool_window * window, btRigidBody * body, float velocity)
{
	_window = window;
	_body = body;
	_velocity = velocity;
}

void fps_look::input(float dt)
{
	ivec2 screen_center = _window->center();

	if (_window->in.mouse(glut_pool_window::button::left) && !_enabled)
	{
		_enabled = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutWarpPointer(screen_center.x, screen_center.y);
		return;
	}

	if (_window->in.key_up(27) && _enabled)  // esc
	{
		_enabled = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (!_enabled)
		return;

	float const angular_movement = 0.1f;
	ivec2 delta = _window->in.mouse_position() - screen_center;

	if (delta.x != 0)
	{
		float angle = radians(angular_movement * delta.x);
		btTransform transf = _body->getCenterOfMassTransform();
		btQuaternion r = btQuaternion{btVector3{0,1,0}, -angle} * transf.getRotation();
		_body->setCenterOfMassTransform(btTransform{r, transf.getOrigin()});
	}

	if (delta.y != 0)
	{
		float angle = radians(angular_movement * delta.y);
		btTransform transf = _body->getCenterOfMassTransform();
		btVector3 right_dir = transf.getBasis().getColumn(0);
		btQuaternion r = btQuaternion{right_dir, -angle} * transf.getRotation();
		_body->setCenterOfMassTransform(btTransform{r, transf.getOrigin()});
	}

	if (delta.x != 0 || delta.y != 0)
		glutWarpPointer(screen_center.x, screen_center.y);
}

void fps_player::init(vec3 const & position, float fovy, float aspect_ratio, float near, float far, glut_pool_window * window)
{
	_cam = camera{fovy, aspect_ratio, near, far};

	float const mass = 90.0f;
	_collision = body_object{make_box_shape(btVector3{0.25, 0.25, 0.25}), mass, btVector3{0, 0.5, 0} + bullet_cast(position)};
	_collision.native()->setAngularFactor(0);  // nechcem aby sa hrac otacal pri kolizii
	_collision.native()->setActivationState(DISABLE_DEACTIVATION);

	_window = window;
	_look.init(_window, _collision.native());
	_move.init(&_window->in, _collision.native());
}

void fps_player::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
}

void fps_player::link_with(rigid_body_world & world, int mark)
{
	world.link(_collision);
	_collision.native()->setGravity(btVector3{0,0,0});  // turn off object gravity
	if (mark != -1)
		_collision.native()->setUserIndex(mark);
}

void fps_player::update(float dt)
{
	_cam.position = glm_cast(_collision.position());
	_cam.rotation = glm_cast(_collision.rotation());
}

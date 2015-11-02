//! implmentuje herny objekt dvere
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "physics/physics.hpp"
#include "fsm.hpp"

enum class door_states {closed, openning, open, closing};

class door;

struct door_closed : public state_machine::state
{};

class door_openning : public state_machine::state
{
public:
	door_openning() {}  //!< makes uninitialized object
	door_openning(door * d) : _door{d} {}
	void enter() override;
	void update(float dt) override;
	void exit() override;

private:
	door * _door = nullptr;
	float DURATION = 1.0f;
};

class door_open : public state_machine::state
{
public:
	door_open() {}
	door_open(door * d) : _door{d} {}
	void update(float dt) override;

private:
	float _t = 0;
	door * _door = nullptr;
	float DURATION = 3.0f;
};

class door_closing : public state_machine::state
{
public:
	door_closing() {}
	door_closing(door * d) : _door{d} {}
	void enter() override;
	void update(float dt) override;
	void exit() override;

private:
	door * _door = nullptr;
	float DURATION = 1.0f;
};

class door_state_machine : public state_machine
{
public:
	door_state_machine() {}
	void init(door * d);
	void enter_openning_sequence();

	door_state_machine(door_state_machine const &) = delete;
	door_state_machine & operator=(door_state_machine const &) = delete;

private:
	void change_state(int state_type_id) override;

	door_closed _closed;
	door_openning _openning;
	door_open _open;
	door_closing _closing;

	door * _door;
};

class door
{
public:
	enum type {horizontal, vertical};

	door(btVector3 const & position, type orientation, gl::mesh const & m, texture2d & diff_tex);
	void update(float dt);
	void render(shader::program & p, glm::mat4 const & world_to_screen);
	void link_with(rigid_body_world & world);  // const
	void open();
	bool can_close() const {return true;}

	// low level
	physics_object & collision() {return _collision;}
	btVector3 open_position() const;
	btVector3 const & closed_position() const;
	type orientation() const;

	door(door const &) = delete;
	door & operator=(door const &) = delete;

private:
	physics_object _collision;
	door_state_machine _state;
	btVector3 _closed_pos;
	gl::mesh const * _mesh;
	texture2d * _diff_tex;  // TODO: make const
	type _orient;
};

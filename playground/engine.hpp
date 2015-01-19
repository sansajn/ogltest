#pragma once
#include "ptr.hpp"
#include "program.hpp"
#include "math.hpp"

class game_object;
class renderer;
class camera;

/*! Definuje chovanie objektov hry.
\sa game_object */
class game_component
{
public:
	virtual ~game_component() {}

	virtual void input(float dt) {}
	virtual void update(float dt) {}
	virtual void render(shader::program & prog, renderer & rend) {}

	transform & transformation();
	transform const & transformation() const;

protected:
	void owner(game_object * o) {_owner = o;}

	game_object * _owner;

	friend class game_object;
};

/*! Predstavuje objekty v hre. */
class game_object
{
public:
	transform transformation;

	game_object();
	virtual ~game_object();

	// object prebera vlastnictvo potomka a komponenty
	void append_child(game_object * child);
	void append_component(game_component * comp);

	void input(float dt);
	void update(float dt);
	void render(shader::program & prog, renderer & rend);

	glm::mat4 const & world_to_local() const;
	glm::mat4 const & local_to_camera() const {return _local_to_camera;}
	glm::mat4 const & local_to_screen() const {return _local_to_screen;}

protected:
	void update_local_to_world(glm::mat4 const & parent_to_world);
	void update_local_to_camera(glm::mat4 const & world_to_camera, glm::mat4 const & camera_to_screen);

private:
	std::vector<game_object *> _children;
	std::vector<game_component *> _components;

	glm::mat4 _local_to_parent;
	glm::mat4 _local_to_world;
	glm::mat4 _local_to_camera;
	glm::mat4 _local_to_screen;
	mutable glm::mat4 _world_to_local;  //!< inv of _local_to_world
	mutable bool _world_to_local_update;

	friend class game_scene;
};

class game_scene
{
public:
	game_scene() : _camera(nullptr) {}

	void append_object(game_object * o) {_root.append_child(o);}
	game_object & root_object() {return _root;}
	game_object * camera_object() const;
	void camera_object(game_object * cam) {_camera = cam;}
	glm::mat4 const & camera_to_screen() const {return _camera_to_screen;}
	void camera_to_screen(glm::mat4 const & m) {_camera_to_screen = m;}

	void input(float dt) {_root.input(dt);}
	void update(float dt);
	void render(shader::program & prog, renderer & rend) {_root.render(prog, rend);}

private:
	game_object _root;
	game_object * _camera;
	glm::mat4 _camera_to_screen;
};

class renderer  //!< implementuje tzv. forward renderer
{
public:
	renderer();

	shader::program & active_program() {return _prog;}

	void render(game_scene & objects);

private:
	shader::program _prog;
};

#pragma once
#include "program.hpp"
#include "math.hpp"

class game_object;

/*! Definuje chovanie objektov hry.
\sa game_object */
class game_component
{
public:
	virtual ~game_component() {}

	virtual void input(float dt);
	virtual void update(float dt);
	virtual void render(shader::program & prog, renderer & rend);

private:
	void patent(game_object * o) {_parent = o;}

	game_object * _parent;

	friend class game_object;
};

/*! Predstavuje objekty v hre. */
class game_object
{
public:
	game_object();
	virtual ~game_object();

	void append_child(game_object * child);
	void append_component(game_component * comp);

	void input(float delta);
	void update(float delta);
	void render(shader::program & prog, renderer & rend);

	transform & transformation() {return _trans;}

private:
	std::vector<game_object *> _children;
	std::vector<game_component *> _components;
	transform _trans;
};

class renderer
{
public:
	renderer();
	~renderer();

	void render(game_object & o);  // render_hierarchy

private:
	shader::program _prog;
};

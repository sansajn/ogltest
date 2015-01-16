#include "engine.hpp"
#include <GL/glew.h>

void game_object::append_child(game_object * child)
{
	_children.push_back(child);
}

void game_object::append_component(game_component * comp)
{
	_components.push_back(comp);
	comp->patent(this);
}

void game_object::input(float delta)
{
	for (game_object * ch : _children)
		ch->input(delta);

	for (game_component * c : _components)
		c->input(delta);
}

void game_object::update(float delta)
{
	for (game_object * ch : _children)
		ch->update(delta);

	for (game_component * c : _components)
		c->update(delta);
}

void game_object::render(shader::program & prog, renderer & rend)
{
	for (game_object * ch : _children)
		ch->render(prog, rend);

	for (game_component * c : _components)
		c->render(prog, rend);
}

renderer::renderer()
{
	glClearColor(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
}

void renderer::render(game_object & o)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	o.render(_prog, this);

	// forward rendering technique
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	// TODO: render lights

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
}

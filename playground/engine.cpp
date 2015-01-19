#include "engine.hpp"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

transform & game_component::transformation()
{
	assert(_owner && "game-component is not assigned to game-object");
	return _owner->transformation;
}

transform const & game_component::transformation() const
{
	assert(_owner && "game-component is not assigned to game-object");
	return _owner->transformation;
}

game_object * game_scene::camera_object() const
{
	assert(_camera && "camera object not set");
	return _camera;
}

void game_scene::update(float dt)
{
	_root.update(dt);
	_root.update_local_to_world(glm::mat4(1.0f));
	_root.update_local_to_camera(camera_object()->world_to_local(), camera_to_screen());
}

game_object::game_object()
	: _world_to_local_update(false)
{}

glm::mat4 const & game_object::world_to_local() const
{
	if (!_world_to_local_update)
	{
		_world_to_local = glm::inverse(_local_to_world);
		_world_to_local_update = true;
	}
	return _world_to_local;
}

void game_object::update_local_to_world(glm::mat4 const & parent_to_world)
{
	_local_to_parent = transformation.transformation();
	_local_to_world = parent_to_world * _local_to_parent;
	for (game_object * ch : _children)
		ch->update_local_to_world(_local_to_world);
}

void game_object::update_local_to_camera(glm::mat4 const & world_to_camera, glm::mat4 const & camera_to_screen)
{
	_local_to_camera = world_to_camera * _local_to_world;
	_local_to_screen = camera_to_screen * _local_to_camera;
	for (game_object * ch : _children)
		ch->update_local_to_camera(world_to_camera, camera_to_screen);
	_world_to_local_update = false;
}

game_object::~game_object()
{
	for (auto c : _components)
		delete c;

	for (auto ch : _children)
		delete ch;
}

void game_object::append_child(game_object * child)
{
	_children.push_back(child);
}

void game_object::append_component(game_component * comp)
{
	_components.push_back(comp);
	comp->owner(this);
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

renderer::renderer() : _prog("directional_light.glsl")
{
	glClearColor(0, 0, 0, 0);
//	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
}

void renderer::render(game_scene & objects)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	objects.render(_prog, *this);

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

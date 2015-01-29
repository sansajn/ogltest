#include "engine.hpp"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include "light.hpp"

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

void game_scene::append_object(game_object * o)
{
	_root.append_child(o);
	o->owner(this);
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

void game_scene::render(renderer & rend)
{
	rend.render(_root);
}

game_object::game_object()
	: _world_to_local_up_to_date(false), _owner(nullptr)
{}

game_object::game_object(glm::vec3 const & pos, glm::quat const & rot, glm::vec3 const & scale)
	: _world_to_local_up_to_date(false), _owner(nullptr), transformation(pos, rot, scale)
{}

glm::mat4 const & game_object::world_to_local() const
{
	if (!_world_to_local_up_to_date)
	{
		_world_to_local = glm::inverse(_local_to_world);
		_world_to_local_up_to_date = true;
	}
	return _world_to_local;
}

game_scene & game_object::owner()
{
	assert(_owner && "unknown game-object owner (not yet benn assigned to game-scene)");
	return *_owner;
}

void game_object::append_to_engine(engine & e)
{
	for (game_object * o : _children)
		o->append_to_engine(e);

	for (game_component * c : _components)
		c->append_to_engine(e);
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
	_world_to_local_up_to_date = false;
}

void game_object::owner(game_scene * s)
{
	_owner = s;
	for (auto ch : _children)
		ch->owner(s);
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

void game_object::render(game_shader & shader, renderer & rend)
{
	for (game_object * ch : _children)
		ch->render(shader, rend);

	for (game_component * c : _components)
		c->render(shader, rend);
}

renderer::renderer()
{
	_shaders.push_back(new ambient_shader);

	glClearColor(0, 0, 0, 0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
}

renderer::~renderer()
{
	for (game_shader * s : _shaders)
		delete s;
	_shaders.clear();
}

void renderer::render(game_object & root)
{
	// forward rendering technique
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	root.render(*_shaders[0], *this);  // ambient

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	for (int i = 1; i < _shaders.size(); ++i)
		root.render(*_shaders[i], *this);  // other lights

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
}

material::material(ptr<texture> diffuse, float specular_intensity, float specular_power)
	: material(diffuse, nullptr, specular_intensity, specular_power)
{}

material::material(ptr<texture> diffuse, ptr<texture> normalmap, float specular_intensity, float specular_power)
	: material(diffuse, normalmap, nullptr, specular_intensity, specular_power)
{}

material::material(ptr<texture> diffuse, ptr<texture> normalmap, ptr<texture> heightmap, float specular_intensity, float specular_power, float disp_scale, float disp_bias)
{
	assoc_texture("diffuse", diffuse);

	if (normalmap)
		assoc_texture("normalmap", normalmap);
	else
		assoc_texture("normalmap", make_ptr<texture>("default_n.png"));

	if (heightmap)
	{
		assoc_float("disp_bias", disp_bias);
		assoc_float("disp_scale", disp_scale);
		assoc_texture("heightmap", heightmap);
	}
	else
	{
		assoc_float("disp_bias", 0.0f);
		assoc_float("disp_scale", 0.0f);
		assoc_texture("heightmap", ptr<texture>(new texture("default_h.png")));
	}

	assoc_float("specular_power", specular_power);
	assoc_float("specular_intensity", specular_intensity);
}

float material::get_float(std::string const & name) const
{
	auto it = _floats.find(name);
	assert(it != _floats.end() && "undefined property");
	return it->second;
}

glm::vec3 material::get_vector(std::string const & name) const
{
	auto it = _vectors.find(name);
	assert(it != _vectors.end() && "undefined property");
	return it->second;
}

ptr<texture> material::get_texture(std::string const & name) const
{
	auto it = _textures.find(name);
	assert(it != _textures.end() && "undefined property");
	return it->second;
}

bool material::find(std::string const & name, float & val) const
{
	auto it = _floats.find(name);
	if (it != _floats.end())
	{
		val = it->second;
		return true;
	}
	else
		return false;
}

bool material::find(std::string const & name, glm::vec3 & val) const
{
	auto it = _vectors.find(name);
	if (it != _vectors.end())
	{
		val = it->second;
		return true;
	}
	else
		return false;
}

bool material::find(std::string const & name, ptr<texture> & val) const
{
	auto it = _textures.find(name);
	if (it != _textures.end())
	{
		val = it->second;
		return true;
	}
	else
		return false;
}

void engine::render()
{
	_scene.render(_rend);
}

void engine::append(game_object * obj)
{
	_scene.append_object(obj);
	obj->append_to_engine(*this);
}

void engine::append_light(game_shader * s)
{
	_rend.append_light(s);
}

void engine::camera_object(game_object * cam, glm::mat4 const & projection)
{
	_scene.camera_object(cam);
	_scene.camera_to_screen(projection);
}

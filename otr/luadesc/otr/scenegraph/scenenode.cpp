#include "scenegraph/scenenode.hpp"
#include <glm/matrix.hpp>
#include "scenegraph/scene.hpp"


void scene_node::append_flag(std::string const & flag)
{
	_flags.insert(flag);
}

void scene_node::assoc_method(std::string const & name, ptr<method> m)
{
	auto it = _methods.find(name);
	if (it == _methods.end())
		_methods[name] = m;
	else
	{
		it->second->_owner = nullptr;
		it->second = m;
	}
	m->_owner = shared_from_this();
}

ptr<method> scene_node::get_method(std::string const & name) const
{
	auto it = _methods.find(name);
	if (it != _methods.end())
		return it->second;
	else
		return ptr<method>();
}

void scene_node::remove_method(std::string const & name)
{
	auto it = _methods.find(name);
	if (it != _methods.end())
	{
		it->second->_owner = nullptr;
		_methods.erase(it);
	}
}

void scene_node::append_child(ptr<scene_node> n)
{
	if (!n->_owner)
	{
		_children.push_back(n);
		n->owner(_owner);
		if (_owner)
			_owner->clear_node_map();
	}
}

void scene_node::assoc_mesh(std::string const & name, ptr<mesh_buffers> m)
{
	_meshes[name] = m;
}

ptr<mesh_buffers> scene_node::get_mesh(std::string const & name) const
{
	auto it = _meshes.find(name);
	assert(it != _meshes.end() && "mesh not found");
	return it->second;
}

ptr<shader::module> scene_node::get_module(std::string const & name)
{
	auto it = _modules.find(name);
	return it != _modules.end() ? it->second : ptr<shader::module>();
}

ptr<any_value> scene_node::value(std::string const & name)
{
	auto it = _values.find(name);
	return it != _values.end() ? it->second : ptr<any_value>();
}

glm::mat4 const & scene_node::world_to_local() const
{
	if (!_world_to_local_up_to_date)
	{
		_world_to_local = glm::inverse(_local_to_world);
		_world_to_local_up_to_date = true;
	}

	return _world_to_local;
}

void scene_node::update_local_to_world(glm::mat4 const & parent_local_to_world)
{
	_local_to_world = parent_local_to_world * _local_to_parent;

	// ekvivalent vyrazu mat4*vec3
	glm::vec3 p = glm::mat3(_local_to_world) * glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 r3 = _local_to_world[3];
	float w_inv = 1.0f/(p.x*r3[0] + p.y*r3[1] + p.z*r3[2] + r3[3]);
	_world_pos = p*w_inv;

	for (ptr<scene_node> ch : _children)
		ch->update_local_to_world(_local_to_world);

	_world_to_local_up_to_date = false;
}

void scene_node::update_local_to_camera(glm::mat4 const & world_to_camera,
	glm::mat4 const & camera_to_screen)
{
	_local_to_camera = world_to_camera * _local_to_world;
	_local_to_screen = camera_to_screen * _local_to_camera;

	for (ptr<scene_node> ch : _children)
		ch->update_local_to_camera(world_to_camera, camera_to_screen);
}

void scene_node::owner(scene_manager * o)
{
	_owner = o;
	for (ptr<scene_node> ch : _children)
		ch->owner(o);
}

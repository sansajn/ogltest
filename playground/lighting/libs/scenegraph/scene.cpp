#include "scenegraph/scene.hpp"
#include <glm/matrix.hpp>


ptr<framebuffer> scene_manager::CURRENTFB = make_ptr<framebuffer>();
ptr<shader_program> scene_manager::CURRENTPROG = make_ptr<shader_program>();


void scene_manager::root(ptr<scene_node> n)
{
	_root = n;
	n->owner(this);
	_camera.reset();
}

scene_manager::node_range scene_manager::nodes(std::string const & flag)
{
	if (_nodes.empty() && _root)
		build_node_map(_root);
	auto p = _nodes.equal_range(flag);
	return boost::make_iterator_range(p.first, p.second);
}

ptr<scene_node> scene_manager::node_variable(std::string const & name) const
{
	auto it = _variables.find(name);
	return it != _variables.end() ? it->second : nullptr;
}

void scene_manager::update(double t, double dt)
{
	_t = t;
	_dt = dt;

	if (!_root)
		return;

	_root->update_local_to_world(glm::mat4(1.0f));
	_world_to_screen = camera_to_screen() * camera_node()->world_to_local();
	_root->update_local_to_camera(camera_node()->world_to_local(), camera_to_screen());
}

void scene_manager::draw()
{
	if (_camera && !_camera_method.empty())
	{
		ptr<method> m = _camera->get_method(_camera_method);
		if (m)
		{
			ptr<task> t = m->create_task();
			if (t)
				_sched->run(t);
			assert(t && "unable to create a task");
		}
		assert(m && "method not found");
	}

	++_nframe;

	assert(_camera && "camera node is not set");
	assert(!_camera_method.empty() && "camera method is not set");
}

void scene_manager::build_node_map(ptr<scene_node> node)
{
	for (auto const & flag : node->flags())
		_nodes.insert(std::make_pair(flag, node));

	if (node->has_child())
		for (auto child : node->children())
			build_node_map(child);
}

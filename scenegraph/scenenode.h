#pragma once
#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <boost/range.hpp>
#include <glm/glm.hpp>
#include "ptr.h"
#include "method.h"
#include "meshbuffers.h"

class scene_manager;  // fwd

/*! Uzol sceny a sucastne implamentacia grafu sceny. */
class scene_node
{
public:
	typedef typename boost::iterator_range<std::set<std::string>::const_iterator> flag_crange_type;
	typedef typename boost::iterator_range<std::vector<ptr<scene_node>>::const_iterator> children_crange_type;

	scene_node() : _owner(nullptr) {}

	void append_child(ptr<scene_node> n);
	children_crange_type children() const {return boost::make_iterator_range(_children);}
	bool has_child() const {return !_children.empty();}

	void append_flag(std::string const & flag);
	flag_crange_type flags() const {return boost::make_iterator_range(_flags);}

	void assoc_method(std::string const & name, ptr<method> m);
	ptr<method> get_method(std::string const & name) const;

	void assoc_mesh(std::string const & name, ptr<mesh_buffers> m);
	ptr<mesh_buffers> get_mesh(std::string const & name) const;	

	glm::mat4 const & local_to_parent() const {return _local_to_parent;}
	void local_to_parent(glm::mat4 const & m) {_local_to_parent = m;}
	glm::mat4 const & local_to_world() const {return _local_to_world;}
	glm::mat4 const & world_to_local() const;
	glm::mat4 const & local_to_camera() const {return _local_to_camera;}
	glm::mat4 const & local_to_screen() const {return _local_to_screen;}

	scene_manager * owner() const {return _owner;}

	// pristupne len zo scene_manager::update()
	void update_local_to_world(glm::mat4 const & parent_local_to_world);
	void update_local_to_camera(
		glm::mat4 const & world_to_camera, glm::mat4 const & camera_to_screen);

private:
	void owner(scene_manager * o);

	std::vector<ptr<scene_node>> _children;
	std::set<std::string> _flags;
	std::map<std::string, ptr<mesh_buffers>> _meshes;	
	std::map<std::string, ptr<method>> _methods;

	glm::mat4 _local_to_parent;
	glm::mat4 _local_to_world;	
	glm::mat4 _local_to_camera;
	glm::mat4 _local_to_screen;
	mutable glm::mat4 _world_to_local;
	mutable bool _world_to_local_up_to_date;

	scene_manager * _owner;

	friend scene_manager;  // nastavuje vlastnika uzla
};

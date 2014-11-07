#pragma once
#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <boost/any.hpp>
#include <boost/range.hpp>
#include <glm/mat4x4.hpp>
#include "core/ptr.hpp"
#include "scenegraph/method.hpp"
#include "render/module.hpp"
#include "render/meshbuffers.hpp"

class scene_manager;  // fwd

/*! Uzol sceny a sucastne implamentacia grafu sceny.
@ingroup scenegraph */
class scene_node : public std::enable_shared_from_this<scene_node>
{
public:
	typedef typename boost::iterator_range<std::set<std::string>::const_iterator> flag_range;
	typedef typename boost::iterator_range<std::vector<ptr<scene_node>>::const_iterator> child_range;
	typedef typename boost::iterator_range<std::map<std::string, ptr<any_value>>::const_iterator> value_range;

	scene_node() : _owner(nullptr) {}

	void append_child(ptr<scene_node> n);
	child_range children() const {return boost::make_iterator_range(_children);}
	bool has_child() const {return !_children.empty();}

	void append_flag(std::string const & flag);
	flag_range flags() const {return boost::make_iterator_range(_flags);}

	void assoc_method(std::string const & name, ptr<method> m);
	ptr<method> get_method(std::string const & name) const;
	void remove_method(std::string const & name);

	void assoc_mesh(std::string const & name, ptr<mesh_buffers> m);
	ptr<mesh_buffers> get_mesh(std::string const & name) const;

	ptr<module> get_module(std::string const & name);  // TODO: premenuj na module
	void assoc_module(std::string const & name, ptr<module> m) {_modules[name] = m;}
	void remove_module(std::string const & name) {_modules.erase(name);}

	ptr<any_value> value(std::string const & name);
	value_range values() const {return boost::make_iterator_range(_values);}  //!< vracia konstantnu dvojicu kluc-hodnota
	void append_value(ptr<any_value> v) {_values[v->name()] = v;}

	template <typename T>
	void assoc_field(std::string const & name, ptr<T> f) {_fields[name] = boost::any(f);}

	template <typename T>
	ptr<T> field(std::string const & name) const;

	glm::mat4 const & local_to_parent() const {return _local_to_parent;}  //!< ekvivalentom modelovej matice
	void local_to_parent(glm::mat4 const & m) {_local_to_parent = m;}
	glm::mat4 const & local_to_world() const {return _local_to_world;}
	glm::mat4 const & world_to_local() const;
	glm::mat4 const & local_to_camera() const {return _local_to_camera;}
	glm::mat4 const & local_to_screen() const {return _local_to_screen;}
	glm::vec3 const & world_pos() const {return _world_pos;}

	scene_manager * owner() const {return _owner;}

	// pristupne len zo scene_manager::update()
	void update_local_to_world(glm::mat4 const & parent_local_to_world);
	void update_local_to_camera(
		glm::mat4 const & world_to_camera, glm::mat4 const & camera_to_screen);

private:
	void owner(scene_manager * o);

	std::vector<ptr<scene_node>> _children;
	std::set<std::string> _flags;
	std::map<std::string, boost::any> _fields;  //!< vseobecne ulozisko pre data uzla
	std::map<std::string, ptr<mesh_buffers>> _meshes;	
	std::map<std::string, ptr<method>> _methods;
	std::map<std::string, ptr<module>> _modules;
	std::map<std::string, ptr<any_value>> _values;  //!< pociatozne hodnoty pre uniformi programu

	glm::mat4 _local_to_parent;
	glm::mat4 _local_to_world;	
	glm::mat4 _local_to_camera;
	glm::mat4 _local_to_screen;
	mutable glm::mat4 _world_to_local;
	mutable bool _world_to_local_up_to_date;
	glm::vec3 _world_pos;

	scene_manager * _owner;  // TODO: raw pointer

	friend class scene_manager;  // nastavuje vlastnika uzla
};


template <typename T>
ptr<T> scene_node::field(std::string const & name) const
{
	auto it = _fields.find(name);
	if (it != _fields.end())
	{
		boost::any const & a = it->second;
		ptr<T> const * p = boost::any_cast<ptr<T>>(&a);
		if (p)
			return *p;
	}

	return ptr<T>();
}

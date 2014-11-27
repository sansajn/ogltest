#pragma once
#include <map>
#include <string>
#include <boost/range.hpp>
#include <glm/mat4x4.hpp>
#include "core/ptr.hpp"
#include "render/program.hpp"
#include "render/framebuffer.hpp"
#include "scenegraph/scenenode.hpp"
#include "taskgraph/scheduler.hpp"
#include "resource/resource_manager.hpp"

/*! pred pouzitim preba zavolat root(), camera_node(), camera_method() a scheduler()
@ingroup scenegraph */
class scene_manager
{
public:
	typedef std::multimap<std::string, ptr<scene_node>> nodemap_t;
	typedef boost::iterator_range<nodemap_t::iterator> node_range;

	scene_manager() : _camera_to_screen(1.0f), _nframe(0) {}
	~scene_manager() {}

	// nodes api
	void root(ptr<scene_node> n);
	ptr<scene_node> root() const {return _root;}
	node_range nodes(std::string const & flag);
	ptr<scene_node> node_variable(std::string const & name) const;
	void node_variable(std::string const & name, ptr<scene_node> node) {_variables[name] = node;}
	void camera_node(std::string const & name);
	void camera_node(ptr<scene_node> n) {_camera = n;}
	ptr<scene_node> camera_node() const {return _camera;}

	void camera_method(std::string const & meth) {_camera_method = meth;}
	void scheduler(ptr<task_scheduler> sched) {_sched = sched;}
	ptr<resource_manager> resources() {return _resman;}
	void resources(ptr<resource_manager> resman) {_resman = resman;}

	void camera_to_screen(glm::mat4 const & m) {_camera_to_screen = m;}  //!< ekvivalent project matice
	glm::mat4 const & camera_to_screen() const {return _camera_to_screen;}
	glm::mat4 const & world_to_screen() const {return _world_to_screen;}

	void update(double t, double dt);
	void draw();

	void clear_node_map() {_nodes.clear();}  // TODO: toto premenuj
	void build_node_map(ptr<scene_node> node);

	static ptr<shader::program> current_program() {return CURRENTPROG;}
	static void current_program(ptr<shader::program> p) {CURRENTPROG = p;}
	static ptr<framebuffer> current_framebuffer() {return CURRENTFB;}

private:
	static ptr<framebuffer> CURRENTFB;
	static ptr<shader::program> CURRENTPROG;

	ptr<scene_node> _root;  // TODO: implementj scene-graph
	ptr<scene_node> _camera;
	std::string _camera_method;
	unsigned int _nframe;
	nodemap_t _nodes;
	ptr<task_scheduler> _sched;
	std::map<std::string, ptr<scene_node>> _variables;  // foreach support
	ptr<resource_manager> _resman;

	double _t;  //!< current time in ms
	double _dt; //!< elapsed time in ms since the last call to #update.

	glm::mat4 _camera_to_screen;  //!< projekcna matica
	glm::mat4 _world_to_screen;
};

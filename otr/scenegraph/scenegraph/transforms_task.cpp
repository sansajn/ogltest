#include "scenegraph/transforms_task.hpp"
#include "core/logger.hpp"
#include "scenegraph/scene.hpp"


transforms_task_factory::transforms_task_factory(
	qualified_name const & module_name, char const * ltos, char const * wp) : _module_name(module_name), _ltos(ltos), _wp(wp)
{}

ptr<task> transforms_task_factory::create_task(ptr<scene_node> context)
{
	// TODO: screen-node support

	// find node-module (if not already)
	if (!_module && !_module_name.target.empty())
	{
		ptr<scene_node> target = _module_name.target_node(context);
		_module = target->get_module(_module_name.name);
		if (!_module)
			throw task_exception("SCREENGRAPH", "transforms: cannot find " + _module_name.target + "." + _module_name.name + " module");
	}
	else if (!_module && !_module_name.name.empty())
	{
		_module = context->owner()->resources()->load_resource<module>(_module_name.name);
		if (!_module)
			throw task_exception("SCENEGRAPH", "transforms: cannot find " + _module_name.name + " module");
	}

	return make_ptr<transforms_task>(context, this);
}

void transforms_task_factory::reload_uniforms(program & prog)
{
	bool uniform_found = false;

	if (_ltos)
	{
		_local_to_screen = prog.get_uniform<uniform_matrix4f>(_ltos);
		if (!_local_to_screen)
			wlog("SCENEGRAPH") << "uniform '" << _ltos << "' not found";  // TODO: specify program info
		else
			uniform_found = true;
	}

	if (_wp)
	{
		_world_pos = prog.get_uniform<uniform3f>(_wp);
		if (_world_pos)
			wlog("SCENEGRAPH") << "uniform '" << _wp << "' not found";
		else
			uniform_found = true;
	}

	if (!uniform_found)
		wlog("SCENEGRAPH") << "ani jeden uniform nebol najdeny";
}

bool transforms_task_factory::transforms_task::run()
{
	dlog("SCENEGRAPH") << "transforms";

	ptr<program> __ensure_program_alive;

	program * prog;
	if (_src->_module && !_src->_module->users().empty())
		prog = *_src->_module->users().begin();  // BUG: to ze program stale existuje nie je po jeho ziskani zarucene
	else
	{
		__ensure_program_alive = scene_manager::current_program();
		prog = __ensure_program_alive.get();
	}

	if (!prog)
		return true;

	dlog("SCENEGRAPH") << "transforms " << prog;

	if (prog != _src->_last_prog)
	{
		_src->reload_uniforms(*prog);
		_src->_last_prog = prog;
	}

	if (_src->_local_to_screen)
		_src->_local_to_screen->set_matrix(_context->local_to_screen());

	if (_src->_world_pos)
		_src->_world_pos->set(_context->world_pos());

	assert(glGetError() == GL_NO_ERROR);
	return true;
}

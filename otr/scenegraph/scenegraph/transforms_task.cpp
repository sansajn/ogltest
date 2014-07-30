#include "transforms_task.h"
#include "scenegraph/scene.h"

ptr<task> transforms_task_factory::create_task(ptr<scene_node> context)
{
	return make_ptr<task_impl>(context, shared_from_this());
}

void transforms_task_factory::reload_uniforms(shader_program & prog)
{
	_local_to_screen =
		std::unique_ptr<uniform_variable>(new uniform_variable(_ltos, prog));
}

bool transforms_task_factory::task_impl::run()
{
	ptr<shader_program> prog = scene_manager::current_program();
	if (prog != _src->_last_prog)
	{
		_src->reload_uniforms(*prog);
		_src->_last_prog = prog;
	}

	if (_src->_local_to_screen)
		*(_src->_local_to_screen) = _context->local_to_screen();

	return true;
}

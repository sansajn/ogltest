#include "transforms_task.h"
#include "scenegraph/scene.h"

ptr<task> transforms_task_factory::create_task(ptr<scene_node> context)
{
	return make_ptr<task_impl>(context, shared_from_this());
}

void transforms_task_factory::reload_uniforms(gl::program & prog)
{
	_local_to_screen =
		std::unique_ptr<gl::uniform_t>(new gl::uniform_t(prog.uniform(_ltos)));
}

bool transforms_task_factory::task_impl::run()
{
	ptr<gl::program> prog = scene_manager::current_program();
	if (prog != _src->_last_prog)
	{
		_src->reload_uniforms(*prog);
		_src->_last_prog = prog;
	}

	if (_src->_local_to_screen)
		*(_src->_local_to_screen) = _context->local_to_screen();

	return true;
}

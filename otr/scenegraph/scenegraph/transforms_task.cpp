#include "scenegraph/transforms_task.hpp"
#include "scenegraph/scene.hpp"

ptr<task> transforms_task_factory::create_task(ptr<scene_node> context)
{
	return make_ptr<task_impl>(context, shared_from_this());
}

void transforms_task_factory::reload_uniforms(program & prog)
{
	_local_to_screen = prog.get_uniform<uniform_matrix4f>(_ltos);
}

bool transforms_task_factory::task_impl::run()
{
	ptr<program> prog = scene_manager::current_program();
	if (!prog)
		return true;

	if (prog != _src->_last_prog)
	{
		_src->reload_uniforms(*prog);
		_src->_last_prog = prog;
	}

	if (_src->_local_to_screen)
		_src->_local_to_screen->set_matrix(_context->local_to_screen());

	assert(glGetError() == GL_NO_ERROR);
	return true;
}

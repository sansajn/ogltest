#include "program_task.h"
#include "scenegraph/scene.h"

program_task_factory::program_task_factory(std::vector<std::string> const & module_names)
{
	_p = make_ptr<gl::program>();
	for (auto m : module_names)
		*_p << m;
	_p->link();
}

ptr<task> program_task_factory::create_task(ptr<scene_node>)
{
	return make_ptr<detail::program_task>(_p);
}

namespace detail {

bool program_task::run()
{
	scene_manager::current_program(_p);
	_p->use();
	return true;
}

}  // detail

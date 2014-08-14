#include "scenegraph/program_task.hpp"
#include "scenegraph/scene.hpp"

class program_task : public task
{
public:
	program_task(ptr<shader_program> p) : task(true, 0), _p(p) {}
	bool run();

private:
	ptr<shader_program> _p;
};  // program_task


program_task_factory::program_task_factory(std::vector<std::string> const & module_names)
{
	_p = make_ptr<shader_program>();
	for (auto m : module_names)
		*_p << m;
	_p->link();
}

ptr<task> program_task_factory::create_task(ptr<scene_node>)
{
	return make_ptr<program_task>(_p);
}


bool program_task::run()
{
	scene_manager::current_program(_p);
	_p->use();
	return true;
}

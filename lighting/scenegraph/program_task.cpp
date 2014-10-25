#include "program_task.hpp"
#include <boost/tokenizer.hpp>
#include "scenegraph/scene.hpp"

class program_task : public task
{
public:
	program_task(ptr<shader_program> p) : task(true, 0), _p(p) {}
	bool run();

private:
	ptr<shader_program> _p;
};  // program_task


program_task_factory::program_task_factory(char const * modules)
{
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	std::string s(modules);
	_p = make_ptr<shader_program>();
	for (auto & t : tokenizer(s, boost::char_separator<char>(";")))
		*_p << t;
	_p->link();
}

program_task_factory::program_task_factory(std::vector<std::string> const & modules)
{
	_p = make_ptr<shader_program>();
	for (auto m : modules)
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

#include "program_task.hpp"
#include <boost/tokenizer.hpp>
#include "core/utils.hpp"
#include "core/logger.hpp"
#include "scenegraph/scene.hpp"

class program_task : public task
{
public:
	program_task(ptr<shader::program> p, ptr<scene_node> n) : task(true, 0), _p(p), _node(n) {}
	bool run() override;

private:
	ptr<shader::program> _p;
	ptr<scene_node> _node;
};

void program_task_factory::init(std::vector<qualified_name> const & modules, bool set_uniforms)
{
	_modules = modules;
	_uniforms = set_uniforms;
}

ptr<task> program_task_factory::create_task(ptr<scene_node> context)
{
	// najdi program
	std::string program_name;
	for (auto & qn : _modules)
	{
		ptr<scene_node> t = qn.target_node(context);
		if (t)
		{
			ptr<shader::module> m = t->get_module(qn.name);
			ptr<resource> r = std::dynamic_pointer_cast<resource>(m);
			std::string module_name = t->owner()->resources()->find_key(r);  // TODO: zizskaj name inak (neefektivne)
			program_name += module_name + ";";
		}
		else
			program_name += qn.name + ";";
	}

	// TODO: sort module names

	ptr<shader::program> p = context->owner()->resources()->load_resource<shader::program>(program_name);

	if (!p)
		throw task_exception("SCENEGRAPH", "program_task: cannot find program '" + program_name + "'");

	return make_ptr<program_task>(p, _uniforms ? context : ptr<scene_node>());
}


bool program_task::run()
{
	if (_node)
	{
		for (auto kv : _node->values())
		{
			ptr<any_value> v = kv.second;
			ptr<uniform> u = _p->get_uniform(v->name());
			if (u)
				u->set_value(v);
			else
				debug_log("SCENEGRAPH", "program_task: uniform '" + v->name() + "' not found in a program");
		}
	}

	scene_manager::current_program(_p);
//	_p->set();  // FIXME: program_task nema nastavovat pouzivany program
	return true;
}

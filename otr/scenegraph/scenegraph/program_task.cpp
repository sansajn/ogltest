#include "program_task.hpp"
#include <boost/tokenizer.hpp>
#include "scenegraph/scene.hpp"
#include "core/utils.hpp"

class program_task : public task
{
public:
	program_task(ptr<program> p) : task(true, 0), _p(p) {}
	bool run() override;

private:
	ptr<program> _p;
};  // program_task


ptr<task> program_task_factory::create_task(ptr<scene_node>)
{
	// FIXME: predpoklada existenciu iba jedneho programu (dokial nemám resource manager, tak musím program vytvoriť zakazdým ako chcem vykresliť scénu)

	if (scene_manager::current_program())
		_p = scene_manager::current_program();
	else
	{
		assert(_modules.size() == 2 && "imlementovane iba pre jeden vs a jeden fs");

		std::string sources[2];
		for (auto const & qn : _modules)
		{
			bool is_vertex = qn.name[qn.name.size()-2] == 'v';  // vertex:.vs, fragment:.fs
			if (is_vertex)
				sources[0] = read_file(qn.name.c_str());
			else
				sources[1] = read_file(qn.name.c_str());
		}

		auto m = make_ptr<module>(330, sources[0].c_str(), sources[1].c_str());
		_p = make_ptr<program>(m);
	}

	return make_ptr<program_task>(_p);
}


bool program_task::run()
{
	scene_manager::current_program(_p);
	_p->set();  // TODO: preco je to tu ?
	return true;
}

#include "draw_tweakbar_task.hpp"
#include <GL/glew.h>
#include <AntTweakBar.h>

class draw_tweakbar_task : public task
{
public:
	draw_tweakbar_task() : task(true, 0)
	{}

	bool run() override;
};

bool draw_tweakbar_task::run()
{
	glBindVertexArray(0);  // TODO: docasny hack
	return TwDraw() != 0;
}

ptr<task> draw_tweakbar_factory::create_task(ptr<scene_node> context)
{
	return make_ptr<draw_tweakbar_task>();
}

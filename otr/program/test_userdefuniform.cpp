// !ma sa iba zbuildovat (nie bezat)! - rozsirenie uniformu o uzivatelsky definovany typ
#include "core/ptr.h"
#include "render/program.h"
#include "ui/sdl_window.h"

class texture
{
public:
	texture() {}
	void append_user(GLuint program_id) {}
	void remove_user(GLuint program_id) {}
};

class texture_uniform : public uniform_variable
{
public:
	texture_uniform() : _program_id(0) {}

	texture_uniform(char const * name, shader_program const & prog)
		: uniform_variable(name, prog)
	{
		_program_id = prog.id();
	}

	void link(char const * name, shader_program const & prog)
	{
		_program_id = prog.id();
		uniform_variable::link(name, prog);
	}

	~texture_uniform() {_t->remove_user(_program_id);}

	texture_uniform & operator=(ptr<texture> t)
	{
		_t = t;
		_t->append_user(_program_id);
		return *this;
	}

private:
	ptr<texture> _t;
	GLuint _program_id;
};  // sampler_variable


class app : public gl::sdl_window
{
public:
	app() : gl::sdl_window(parameters().size(800, 600).name("user defined uniform"))
	{
		_prog << "shader/simple.vs" << "shader/simple.fs";
		_prog.link();

		_prog.use();
		_tex_uniform.link("tex", _prog);
	}

	void display() override
	{
		_prog.use();
		ptr<texture> tex(new texture());
		_tex_uniform = tex;
		gl::sdl_window::display();
	}

private:
	shader_program _prog;
	texture_uniform _tex_uniform;
};


int main(int argc, char * argv[])
{
	ptr<app> a(new app());
	a->start();
	return 0;
}

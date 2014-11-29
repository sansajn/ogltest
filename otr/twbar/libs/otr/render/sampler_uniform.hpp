#pragma once
#include "core/ptr.hpp"
#include "render/program.hpp"
#include "render/texture.hpp"

// TODO: removed from source

/*! Umoznuje upload textur do shaderu.
\ingroup render */
//class sampler_uniform : public uniform_variable
//{
//public:
//	sampler_uniform(char const * name, ptr<shader_program> prog)
//		: uniform_variable(name, *prog), _prog(prog)
//	{}

//	~sampler_uniform() {_t->remove_user(_prog->id());}
//	sampler_uniform & operator=(ptr<texture> t);
//	void bind_sampler(ptr<sampler> s) {_s = s;}

//private:
//	ptr<sampler> _s;
//	ptr<texture> _t;
//	ptr<shader_program> _prog;
//};

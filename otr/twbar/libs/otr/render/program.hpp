#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>
#include "core/ptr.hpp"
#include "render/module.hpp"
#include "render/uniform.hpp"

class uniform;
class uniform_sampler;
class texture;
class framebuffer;

namespace shader {

/*! Implementuje shader-program skladajúci sa s modulov.
\saa module
\ingroup render */
class program
{
public:
	program(ptr<module> m);
	program(std::vector<ptr<module>> & modules) {init(modules);}
	virtual ~program();
	int id() const {return _program_id;}
	int module_count() const {return _modules.size();}
	ptr<module> get_module(int idx) const {return _modules[idx];}
	std::vector<ptr<uniform>> uniforms() const;
	ptr<uniform> get_uniform(std::string const & name) const;

	template <typename R = uniform>
	ptr<R> get_uniform(std::string const & name) const
	{
		ptr<uniform> u = get_uniform(name);
		return u ? std::dynamic_pointer_cast<R>(u) : ptr<R>();
	}

protected:
	program() {}
	void init(std::vector<ptr<module>> & modules);
	void init_uniforms();

private:
	bool check_samplers();
	void set();
	void bind_textures();
	void update_texture_users(bool add);
	void update_uniforms(program * owner);
	bool current() const;

	std::vector<ptr<module>> _modules;
	GLuint _program_id;
	std::map<std::string, ptr<uniform>> _uniforms;
	std::vector<ptr<uniform_sampler>> _uniform_samplers;

	static program * CURRENT;

	friend uniform;
	friend uniform_sampler;
	friend texture;
	friend framebuffer;
};

}  // shader

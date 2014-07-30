{Závislosti}

Nemá závislosť na ničom s otr.

{Použitie}
OpenGL shader program utility \saa program. Can be used as [Example:

	// creation
	shader_program prog;
	prog << "simple.vs" << "simple.fs";
	prog.link();

	// in render
	prog.use();
	uniform u("scale", prog);
	u = w/h;
	do_some_render();
	prog.unuse();

--- example end].

Pre rozšírenie uniformu o podporu užívateľsky definovaného typu, stačí
implementovať danný typ ako potomka uniform_variable a definovať mu operator=() takto [Ukážka:

	class texture {};

	class texture_uniform : public uniform_variable
	{
	public:
		using uniform_variable::uniform_variable;

		texture_uniform & operator=(texture const & t) {
			texture_unit = ...	
			glUniform1i(location(), texture_unit);
			return *this;
		}	
	};

	foo() {
		texture tex;
		...
		prog.use();
		texture_uniform u("tex", prog);
		u = tex;
		render();
		prog.unuse();
	}
	
--- koniec ukážky].

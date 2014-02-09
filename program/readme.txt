OpenGL program shader utility \saa program. Can be used as [Example:

	// creation
	gl::program prog;
	prog << "simple.vs" << "simple.fs";
	prog.link();

	// in render
	prog.use();
	do_some_render();
	prog.unuse();

--- example end].

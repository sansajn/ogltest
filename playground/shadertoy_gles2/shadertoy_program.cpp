#include "shadertoy_program.hpp"

using std::string;
using glm::vec2;

shadertoy_program::shadertoy_program()
{}

shadertoy_program::shadertoy_program(string const & fname)
{
	load(fname);
}

bool shadertoy_program::load(string const & fname)
{
	string prolog = R"(
		#ifdef _VERTEX_
		attribute vec3 position;
		void main() {
			gl_Position = vec4(position, 1);
		}
		#endif
		#ifdef _FRAGMENT_
		precision mediump float;
		uniform float iTime;
		uniform vec2 iResolution;
	)";

	string mainImage = gles2::shader::read_file(fname);

	string epilog = R"(
		void main() {
			mainImage(gl_FragColor, gl_FragCoord.xy);
		}
		#endif
	)";

	try {
		_prog.free();
		_prog.from_memory(prolog + mainImage + epilog, 100);
	} catch (std::exception e) {
		return false;
	}

	return true;
}

void shadertoy_program::use()
{
	_prog.use();
	_time_u = _prog.uniform_variable("iTime");
	_resolution_u = _prog.uniform_variable("iResolution");
}

void shadertoy_program::update(float t, vec2 const & resolution)
{
	*_time_u = t;
	*_resolution_u = resolution;
}

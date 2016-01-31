#include "default_shaders.hpp"

namespace shader {

using std::shared_ptr;


// shader bez osvetlenia modelu, model ma len farbu
char const * flat_shader_source = R"(
	// #version 330
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(.7);
	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	void main() {
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main() {
		fcolor = vec4(color, 1);
	}
	#endif
)";

// zozbrazi model s tienovanim zalozenom na normale vrchola (diffuse zlozka phong osvetlenia)
char const * flat_shaded_shader_source = R"(
	// #version 330
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,2,3));

#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main() {
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
#endif

#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	void main() {
		fcolor = vec4(max(dot(n, light_dir), 0.2) * color, 1);
	}
#endif
)";

char const * flat_phong_shader_source = R"()";
char const * textured_shader_source = R"()";
char const * textured_phong_shader_source = R"()";

// pomocou geometry shadera vyzualizuje normaly v kazdom vrchole modelu
char const * normal_geometry_shader_source = R"(
	uniform mat4 local_to_screen;
	uniform float normal_length = 0.01;

	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 2) in vec3 normal;

	out Vertex {
		vec3 normal;
	} vertex;

	void main()
	{
		vertex.normal = normal;
		gl_Position = vec4(position,1);
	}
	#endif  // _VERTEX_

	#ifdef _GEOMETRY_
	layout(triangles) in;
	layout(line_strip, max_vertices = 6) out;

	in Vertex {
		vec3 normal;
	} vertex[];

	void main()
	{
		for (int i = 0; i < gl_in.length(); ++i)
		{
			vec3 p = gl_in[i].gl_Position.xyz;
			gl_Position = local_to_screen * vec4(p,1);
			EmitVertex();
			
			vec3 n = vertex[i].normal.xyz;
			gl_Position = local_to_screen * vec4(p + n*normal_length, 1);
			EmitVertex();
			
			EndPrimitive();
		}
	}
	#endif  // _GEOMETRY_

	#ifdef _FRAGMENT_
	out vec4 color;

	void main()
	{
		color = vec4(0,0,1,1);
	}
	#endif  // _FRAGMENT_
)";


// pomocou geometry shadera vyzualizuje tangent vektory v kazdom vrchole modelu
char const * tangent_geometry_shader_source = R"(
	uniform mat4 local_to_screen;
	uniform float tangent_length = 0.01;

	#ifdef _VERTEX_
	layout(location = 0) in vec3 position;
	layout(location = 3) in vec3 tangent;

	out Vertex {
		vec3 tangent;
	} vertex;

	void main()
	{
		vertex.tangent = tangent;
		gl_Position = vec4(position,1);
	}
	#endif  // _VERTEX_

	#ifdef _GEOMETRY_
	layout(triangles) in;
	layout(line_strip, max_vertices = 6) out;

	in Vertex {
		vec3 tangent;
	} vertex[];

	void main()
	{
		for (int i = 0; i < gl_in.length(); ++i)
		{
			vec3 p = gl_in[i].gl_Position.xyz;
			gl_Position = local_to_screen * vec4(p,1);
			EmitVertex();

			vec3 t = vertex[i].tangent.xyz;
			gl_Position = local_to_screen * vec4(p + t*tangent_length, 1);
			EmitVertex();

			EndPrimitive();
		}
	}
	#endif  // _GEOMETRY_

	#ifdef _FRAGMENT_
	out vec4 color;

	void main()
	{
		color = vec4(1,0,0,1);
	}
	#endif  // _FRAGMENT_
)";


char const * default_shader_impl_list[] = {
	flat_shader_source,
	flat_shaded_shader_source,
	flat_phong_shader_source,
	textured_shader_source,
	textured_phong_shader_source,
	normal_geometry_shader_source,
	tangent_geometry_shader_source
};


shared_ptr<program> default_program_manager::get(default_program_id pid)
{
	auto it = _shaders.find(pid);
	if (it != _shaders.end())
		return it->second;  // already in cache, return it

	// program doesn't exist yet, create
	shared_ptr<program> prog{new program{}};
	prog->from_memory(default_shader_impl_list[(int)pid]);
	_shaders[pid] = prog;
	return prog;
}


default_program_manager & default_shaders()
{
	static default_program_manager * pman = nullptr;
	if (!pman)
	{
		pman = new default_program_manager{};
		resource_manager_list::ref().append(pman);
	}
	return *pman;
}


}  // shader

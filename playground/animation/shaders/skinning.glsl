// implementacia linear blend skinning-u

const int MAX_JOINTS = 100;

uniform mat4 local_to_screen;
uniform mat3 normal_to_world;
uniform vec3 color = vec3(0.5, 0.5, 0.5);
uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako hierarchia transformacii

#ifdef _VERTEX_

layout(location = 0) in vec3 position;  // bind pose positions
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
// 3 for tangent
layout(location = 4) in ivec4 joints;
layout(location = 5) in vec4 weights;

out VS_OUT {
	vec2 uv;
	vec3 normal;
} vs_out;  // vertex shader outputs

void main()
{
	vs_out.uv = texcoord;

	mat4 T_skin =
		skeleton[joints.x] * weights.x +
		skeleton[joints.y] * weights.y +
		skeleton[joints.z] * weights.z +
		skeleton[joints.w] * weights.w;

	vec4 normal_ = T_skin * vec4(normal, 0);  // TODO: prejavy sa posunutie v nasobeni ?
	vs_out.normal = normal_to_world * normal_.xyz;

	gl_Position = local_to_screen * T_skin * vec4(position, 1);
}

#endif  // _VERTEX_

#ifdef _FRAGMENT_

in VS_OUT {
	vec2 uv;
	vec3 normal;
} fs_in;  // fragment shader inputs

out vec4 fcolor;

vec3 light_direction = normalize(vec3(1,1,1));

void main()
{
	// do simple lighting based on normals
	float light_intensity = clamp(dot(normalize(fs_in.normal), light_direction), 0.4, 1);

	vec3 c;
	if (gl_FrontFacing)
		c = vec3(0,1,0);
	else
		c = vec3(1,0,0);

	fcolor = vec4(light_intensity * c, 1);
}

#endif  // _FRAGMENT_

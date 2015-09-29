// implementacia linear blend skinning-u

const int MAX_JOINTS = 100;

uniform mat4 local_to_screen;
uniform mat3 normal_to_world;
uniform vec4 color = vec4(0.5, 0.5, 0.5, 1);
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

//	mat4 T_skin =
//		skeleton[joints.x] * weights.x +
//		skeleton[joints.y] * weights.y +
//		skeleton[joints.z] * weights.z +
//		skeleton[joints.w] * weights.w;

	//	vec4 normal_ = T_skin * vec4(normal, 0);  // TODO: prejavy sa posunutie v nasobeni ?
	//	vs_out.normal = normal_to_world * normal_.xyz;

	vs_out.normal = normal_to_world * normal;  // nebere do uvahu animaciu

	vec4 p_bind = vec4(position, 1);
	vec4 p =
		weights.x * skeleton[joints.x] * p_bind +
		weights.y * skeleton[joints.y] * p_bind +
		weights.z * skeleton[joints.z] * p_bind +
		weights.w * skeleton[joints.w] * p_bind;

	gl_Position = local_to_screen * p;
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
	fcolor = light_intensity * color;
}

#endif  // _FRAGMENT_

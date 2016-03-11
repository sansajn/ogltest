#ifdef _VERTEX_
layout(location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position, 1);
}
#endif

#ifdef _FRAGMENT_

float snoise(vec3 uv, float res)
{
	const vec3 s = vec3(1e0, 1e2, 1e3);
	uv *= res;
	vec3 uv0 = floor(mod(uv, res)) * s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res)) * s;
	
	vec3 f = fract(uv);
	f = f*f*(3.0 - 2.0*f);
	
	vec4 v = vec4(
		uv0.x + uv0.y + uv0.z, uv1.x + uv0.y + uv0.z,
		uv0.x + uv1.y + uv0.z, uv1.x + uv1.y + uv0.z);
	
	vec4 r = fract(sin(v*1e-1)*1e3);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v+uv1.z - uv0.z)*1e-1)*1e3);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.0 - 1.0;
}

uniform vec2 resolution;  // viewport resolution in pixels
uniform float global_time;  // shader playback time in seconds
// uniform float time_delta;  // render time in seconds
// uniform int frame;  // shader playback frame
// channel_time[4]
// channel_resolution[4]
// mouse
// channel0..3
// date
// sample_rate


void main_image(out vec4 frag_color, in vec2 frag_coord)
{
	vec2 p = -.5 + frag_coord.xy / resolution.xy;
	p.x *= resolution.x / resolution.y;
	
	float color = 3.0 - 3.0*length(2.0*p);
	vec3 coord = vec3(atan(p.x, p.y)/6.2832 + 0.5, length(p)*0.4, 0.5);
	
	for (int i = 1; i <= 7; i++)  // medzi 5 a 7 uz neviem rozoznat
	{
		float power = pow(2.0, float(i));
		color += (1.5/power) * snoise(coord + vec3(0.0, -global_time*0.05, global_time*0.01), power*16.0);
	}
	
	frag_color = vec4(color, pow(max(color,0.0), 2.0)*0.4, pow(max(color,0.0), 3.0)*0.15, 1.0);
}

out vec4 fcolor;

void main()
{
	main_image(fcolor, gl_FragCoord.xy);
}

#endif

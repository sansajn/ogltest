// #version 100

#ifdef _VERTEX_
attribute vec3 position;

void main()
{
	gl_Position = vec4(position, 1);
}
#endif

#ifdef _FRAGMENT_

precision mediump float;

uniform float iTime;
uniform vec2 iResolution;

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord/iResolution.xy;

	// Time varying pixel color
	vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

	// Output to screen
	fragColor = vec4(col,1.0);
}

void main()
{
	mainImage(gl_FragColor, gl_FragCoord.xy);
}

#endif

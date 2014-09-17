#version 120

uniform vec2 iResolution;
uniform float iGlobalTime;

varying vec3 raydir;

void main()
{
	vec3 rd = normalize(raydir);
	float tmp = fract(iGlobalTime);
	gl_FragColor = vec4(rd.x, tmp, iResolution.x, 0);
}


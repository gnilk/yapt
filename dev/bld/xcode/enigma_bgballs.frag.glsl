#version 120

uniform vec2 iResolution;

void sampleCamera(vec2 u, out vec3 rayOrigin, out vec3 rayDir)
{
	vec2 filmUv = (gl_FragCoord.xy + u)/iResolution.xy;

	float tx = (2.0*filmUv.x - 1.0)*(iResolution.x/iResolution.y);
	float ty = (1.0 - 2.0*filmUv.y);
	float tz = 0.0;

	// vec2 sc = vec2(cos(iGlobalTime), sin(iGlobalTime));
	// vec3 rd = normalize(vec3(rayOrigin.x * sc.x - sc.y, rayOrigin.y, sc.x + rayOrigin.x * rayOrigin.y));
	// rayOrigin = rd;

	rayOrigin = vec3(0.0, 0.0, 2.0);
	rayDir = normalize(vec3(tx, ty, tz) - rayOrigin);
}



void main(void)
{
	vec3 rayOrigin, rayDir;
	sampleCamera(vec2(0.5,0.5), rayOrigin, rayDir);

	vec2 filmUv = (gl_FragCoord.xy)/iResolution.xy;
	float c = pow(1.0 - filmUv.y, 6);
	if (c < 0) {
		c = 0;
	}
	
	gl_FragColor = vec4(0,0,c,1.0);
}

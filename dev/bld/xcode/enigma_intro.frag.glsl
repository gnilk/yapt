#version 120

varying vec4 baseVertex;

void main(void) 
{
	vec3 col = vec3(1,1,1);
	vec3 v = normalize(baseVertex.xyz);
	float c = v.z;
	if (v.z < 0.25) {
		c = 0.25;
	} 

	col = vec3(c,c,c);
	gl_FragColor = vec4(col, 0);	
}


#version 120

varying vec4 baseVertex;

void main(void) 
{
	vec3 col = vec3(1,0.05,0.05);
	if (baseVertex.z < 0.0) {
		col = vec3(0.35,0.03,0.03);
	} 
	gl_FragColor = vec4(col, 0);	
}


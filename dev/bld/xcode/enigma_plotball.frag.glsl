#version 120

varying vec4 baseVertex;

void main(void) 
{
	vec3 col = vec3(1,1,1);
	if (baseVertex.z < 0.0) {
		col = vec3(0.25,0.25,0.25);
	} 
	gl_FragColor = vec4(col, 0);	
}


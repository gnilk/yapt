#version 120

varying vec4 baseVertex;

void main(void) 
{
	vec4 color = vec4(1,1,1,1);
	vec4 col = color;
	if (baseVertex.z < 0.0) {
		col = color * 0.25; 
	} 
	gl_FragColor = col; //vec4(col, 0);	
}


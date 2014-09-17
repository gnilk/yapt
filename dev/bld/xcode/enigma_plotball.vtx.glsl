#version 120
varying vec4 baseVertex;
void main(void)
{
	mat4 rotMatrix;
	// Take modelview matrix and reset translation and projection stuff
	rotMatrix = gl_ModelViewMatrix;
	// Translation
	rotMatrix[0][3] = 0.0;
	rotMatrix[1][3] = 0.0;
	rotMatrix[2][3] = 0.0;
	// projection
	rotMatrix[3] = vec4(0,0,0,1);

	// Now, only rotate vertex (this is for coloring later on)
	baseVertex = rotMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
} 



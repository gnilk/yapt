#version 120

// attribute vec4 coord;
//varying vec2 texpos;


void main(void)
{
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

//	gl_Position = vec4(coord.xy, 0, 1);
	gl_TexCoord[0] = gl_MultiTexCoord0;
} 



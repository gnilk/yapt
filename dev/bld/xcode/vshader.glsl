#version 120

varying vec3 raydir;

void main()
{
	vec3 r;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	r = gl_Vertex.xyz*vec3(1.3333,1.0,0.0)+vec3(0.0,0.0,-1.0);

	raydir = r;
}



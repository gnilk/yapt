#version 120

uniform sampler2D tex;

void main(void) 
{
//	gl_FragColor = vec4(1,1,1, 0);	
//	gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).a);
	gl_FragColor = texture2D(tex, gl_TexCoord[0].xy);
}


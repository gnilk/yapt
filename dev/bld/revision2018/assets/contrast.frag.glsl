#version 120

uniform sampler2D myTexture;
varying vec2 vTexCoord;
// from program
uniform vec2 iResolution;
uniform float iGlobalTime;
uniform float contrastfactor;
uniform float contrastscale;


void contrast() {
	vec4 col = vec4(0,0,0,0);
	vec2 uv = vTexCoord;

	col = texture2D(myTexture, uv);
	col.r = pow(col.r, contrastfactor)*contrastscale;
	col.g = pow(col.g, contrastfactor)*contrastscale;
	col.b = pow(col.b, contrastfactor)*contrastscale;

	gl_FragColor = col;
}

void passthrough() {
	gl_FragColor = texture2D(myTexture, vTexCoord);

}
void main(void) 
{
	contrast();
	//blur_3x3_kernel();
	//passthrough();
	// gl_FragColor = vec4(1,1,1, 0);	
}


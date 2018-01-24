#version 120

uniform sampler2D myTexture;
varying vec2 vTexCoord;
// from program
uniform vec2 iResolution;
uniform float iGlobalTime;
uniform sampler2D texA;
uniform sampler2D texB;
uniform float blendFactorTexA;
uniform float blendFactorTexB;


void blend() {
	vec4 col = vec4(0,0,0,0);
	vec2 uv = vTexCoord;

	vec4 colA = texture2D(texA, uv);
	vec4 colB = texture2D(texB, uv);
	col = blendFactorTexA*colA + blendFactorTexB*colB;

	gl_FragColor = col;
}

void passthrough() {
	gl_FragColor = texture2D(myTexture, vTexCoord);

}
void main(void) 
{
	blend();
	//blur_3x3_kernel();
	//passthrough();
	// gl_FragColor = vec4(1,1,1, 0);	
}


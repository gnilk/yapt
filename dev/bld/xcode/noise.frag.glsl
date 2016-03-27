#version 120

uniform sampler2D myTexture;
varying vec2 vTexCoord;
// from program
uniform vec2 iResolution;
uniform float iGlobalTime;

uniform float noise_scale = 1.0;
uniform float noise_offset = 0.0;




void passthrough() {
	gl_FragColor = texture2D(myTexture, vTexCoord);

}

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 do_noise() {
	vec4 col = texture2D(myTexture, vTexCoord);
	float r = rand(vTexCoord.xy);
	r = r * noise_scale + noise_offset;
	col = col * r;
	return col;
}

void main(void) 
{
	vec4 col = do_noise();
	gl_FragColor = col;
	//blur_9x9_kernel();
	//blur_3x3_kernel();
	//passthrough();
	// gl_FragColor = vec4(1,1,1, 0);	
}


#version 120

uniform sampler2D myTexture;
varying vec2 vTexCoord;
// from program
uniform vec2 iResolution;
uniform float iGlobalTime;
uniform float blurdist;


void blur_9x9_kernel() {
	// kernel ripped from some tutorial
	float weights[9] = float[](0.0162162162,0.0540540541,0.1216216216,0.1945945946, 0.2270270270 ,0.1945945946,0.1216216216,0.0540540541,0.0162162162);


	vec4 col = vec4(0,0,0,0);
	vec2 uv = vTexCoord;

	float x_step = blurdist/iResolution.x;
	float y_step = blurdist/iResolution.y;

	uv.y = vTexCoord.y - (4 * y_step);
	for (int j=0;j<9;j++) {
		uv.x = vTexCoord.x - (4 * x_step);
		for(int i=0;i<9;i++) {
			col = col + texture2D(myTexture, uv).rgba * weights[i] * weights[j];
			uv.x += x_step;
		}
		uv.y += y_step;
	}
	
	gl_FragColor = col; //texture2D(myTexture, vTexCoord).rgba;

}

void blur_3x3_kernel() {
	float weights[3] = float[](0.25,0.5,0.25);


	vec4 col = vec4(0,0,0,0);
	vec2 uv = vTexCoord;

	float x_step = blurdist/iResolution.x;
	float y_step = blurdist/iResolution.y;

	uv.y = vTexCoord.y - (1 * y_step);
	for (int j=0;j<3;j++) {
		uv.x = vTexCoord.x - (1 * x_step);
		for(int i=0;i<3;i++) {
			col = col + texture2D(myTexture, uv).rgba * weights[i] * weights[j];
			uv.x += x_step;
		}
		uv.y += y_step;
	}

	gl_FragColor = col; //texture2D(myTexture, vTexCoord).rgba;

}

void passthrough() {
	gl_FragColor = texture2D(myTexture, vTexCoord);

}
void main(void) 
{
	//blur_9x9_kernel();
	blur_3x3_kernel();
	//passthrough();
	// gl_FragColor = vec4(1,1,1, 0);	
}


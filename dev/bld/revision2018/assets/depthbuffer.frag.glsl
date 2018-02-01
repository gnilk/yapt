#version 120

uniform sampler2D myTexture;
varying vec2 vTexCoord;
// from program
uniform vec2 iResolution;
uniform float iGlobalTime;

// void main(void) 
// {
// 	vec2 uv = vTexCoord;	
// 	vec4 col = texture2D(myTexture, uv);
// 	gl_FragColor = 	col;
// }


float LinearizeDepth(in vec2 uv)
{
    float zNear = 1.0;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 10.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture2D(myTexture, uv).r;
    float lDepth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
    return (pow(lDepth,16)*1024);
    //return lDepth;
}

float DepthContour(in vec2 uv) {
	float x_step = 4/iResolution.x;
	float y_step = 4/iResolution.y;

	vec2 uvLeft = vec2(uv.x - x_step, uv.y);
	vec2 uvDown = vec2(uv.x, uv.y - y_step);

	float dc = LinearizeDepth(uv);
	float dl = LinearizeDepth(uvLeft);
	float dd = LinearizeDepth(uvDown);

	float dx = abs(dl - dc);
	float dy = abs(dd - dc);


	float c = 0;
	if ((dx > 0.1) || (dy > 0.1)) {
		c = 1.0;
	}

	return c;
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float ColorToLuma(in vec2 uv) {
	vec4 col = texture2D(myTexture, uv);
	vec3 hsv = rgb2hsv(vec3(col.x, col.y, col.z));
	return hsv.z;
}

float Contrast(in float luma) {
	float c = pow(luma,4)*4;
	c = max(0,c);
	return c;
}

float ContrastAt(vec2 uv) {
	float l = ColorToLuma(uv);
	return Contrast(l);
}

float LumaContour(in vec2 uv) {
	float x_step = 1/iResolution.x;
	float y_step = 1/iResolution.y;

	vec2 uvLeft = vec2(uv.x - x_step, uv.y);
	vec2 uvDown = vec2(uv.x, uv.y - y_step);

	float dc = ContrastAt(uv);
	float dl = ContrastAt(uvLeft);
	float dd = ContrastAt(uvDown);

	float dx = abs(dl - dc);
	float dy = abs(dd - dc);


	float c = 0;
	if ((dx > 0.001) || (dy > 0.001)) {
		c = 1.0;
	}

	return c;
}

void main()
{
    // float cc = DepthContour(vTexCoord); //LinearizeDepth(vTexCoord);
    // float cd = LinearizeDepth(vTexCoord); //LinearizeDepth(vTexCoord);
    // gl_FragColor = vec4(cc, cd, 0, 1.0);
    float luma = ColorToLuma(vTexCoord);
    float cc = Contrast(luma);
    float cv = LumaContour(vTexCoord);
    gl_FragColor = vec4(cv, cv, 0, 1.0); //texture2D(myTexture, vTexCoord);
}

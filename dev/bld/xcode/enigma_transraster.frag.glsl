#version 120

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec2 iResolution;
uniform float iGlobalTime;
uniform float raster_offset;
uniform float gamma;

#define PI 3.1415926535898


void sampleCamera(vec2 u, out vec3 rayOrigin, out vec3 rayDir)
{
	vec2 filmUv = (gl_FragCoord.xy + u)/iResolution.xy;

	float tx = (2.0*filmUv.x - 1.0)*(iResolution.x/iResolution.y);
	float ty = (1.0 - 2.0*filmUv.y);
	float tz = 0.0;

	// vec2 sc = vec2(cos(iGlobalTime), sin(iGlobalTime));
	// vec3 rd = normalize(vec3(rayOrigin.x * sc.x - sc.y, rayOrigin.y, sc.x + rayOrigin.x * rayOrigin.y));
	// rayOrigin = rd;

	rayOrigin = vec3(0.0, 0.0, 2.0);
	rayDir = normalize(vec3(tx, ty, tz) - rayOrigin);
}



void main(void)
{
	float t = iGlobalTime;
	vec3 rayOrigin, rayDir;
	sampleCamera(vec2(0.5,0.5), rayOrigin, rayDir);

	vec2 filmUv = (gl_FragCoord.xy)/iResolution.xy;


	vec2 uv1 = gl_TexCoord[0].yx;
	vec2 uv2 = gl_TexCoord[1].xy;

	uv1.x += raster_offset;
	uv1.x = filmUv.y;
	uv1.y = filmUv.x + raster_offset;
	// float tmp = uv1.x;
	// uv1.x = uv1.y;
	// uv1.y = tmp + raster_offset*2;
	//vec4 col = vec4(0,0,0,0);

	uv1.x = filmUv.x;
	uv1.y = filmUv.y;
	float tx = -1*filmUv.y;// + t*0.4;
	uv1.x = tx+raster_offset*0.2;
	
	vec4 col2 = texture2D(texture2, uv2).rgba;
	vec4 col1 = texture2D(texture1, uv1).rgba;

	//float gamma = sin(raster_offset);	// should come from XML

	col1.r = min(col1.r + gamma,1);
	col1.g = min(col1.g + gamma,1);
	col1.b = min(col1.b + gamma,1);

	gl_FragColor = col1*col2; //vec4(r,g,b,1.0);
}

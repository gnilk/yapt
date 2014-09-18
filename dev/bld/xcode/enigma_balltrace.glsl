#version 120


// TODO: Rewrite this one!!

uniform vec2 iResolution;
uniform float iGlobalTime;

vec3 lightPos = normalize(vec3(2,-2,0.5));
vec4 plane = vec4(0,-1,0.0,0.2);

//vec4 plane = vec4(0,0,1,2);

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

float Q(float a, float b, float c)
{
	float d = b*b-4.0*a*c;
	if (d < 0.0) return -1.0;
	d=sqrt(d);	
	float oo2a = 0.5/a;
	return min((-b-d)*oo2a,(-b+d)*oo2a);
}
	

float Lit(vec3 N, vec3 V, vec3 H)
{
	float d = max(dot(N,lightPos),0.0) * 0.5;
	float s = pow(max(dot(N,H),0.0),100.0)*.5;
	
	return d+s;
}


float checkPlane( in vec4 pla, in vec3 ro, in vec3 rd )
{
    float de = dot(pla.xyz, rd);
    de = sign(de)*max( abs(de), 0.001);
    float t = -(dot(pla.xyz, ro) + pla.w)/de;
    return t;
}

vec3 mandelbrot(vec3 planehit) {
	// Created by inigo quilez - iq/2013
	// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

    vec2 p = planehit.xz; //(-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;

    float zoo = .44+.16*cos(.05*iGlobalTime); // 0.44; 
    float coa = 0.1*(1.0-zoo); //cos( 0.1*(1.0-zoo)*iGlobalTime );
    float sia = 0.1*(1.0-zoo); // sin( 0.1*(1.0-zoo)*iGlobalTime );
    zoo = pow( zoo,8.0);
    vec2 xy = vec2( p.x*coa-p.y*sia, p.x*sia+p.y*coa);
    vec2 cc = vec2(-.745,.186) + xy*zoo;

    float co = 0.0;
	vec2 z  = vec2(0.0);
    for( int i=0; i<256; i++ )
    {
        // z = z*z + c		
		z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + cc;
		
		if( dot(z,z)>(256.0*256.0) ) break;

		co += 1.0;
    }

	return vec3( 0.5 + 0.5*cos( 3.0 + co*0.2 + vec3(0.0,0.5,1.0)));
}


vec3 trace(vec3 P, vec3 V, vec3 H)
{
	float time = iGlobalTime * 4;
	vec3 c=vec3(0.0,0.0,0.0);

#define N_SPHERE	24
	
	vec4 S[N_SPHERE];
	vec3 C[N_SPHERE];
	vec3 gb=vec3(0.1, 0.96, 0.1);
	vec3 planeColor=vec3(0.1, 0.1, 0.96);

	if (V.y < 0) {
	  	float py = 1.0 - (-2.0 * V.y);  //P.z * 0.1;

	  	c = vec3(py, 0, 1.0 - py);
	}
	// float py = V.y;
	// c = vec3(py, py, py);
	
	for (int i=0; i<N_SPHERE; i++)
	{
		float I = float(i)*(1.0/float(N_SPHERE));
		float t = I*2.0*3.1415927;
		// position
		//vec3 A = vec3(sin(t),sin(t*3.0)*0.4,cos(t));
		float tt = fract(time);
		float x = fract(time) *(1.0/float(N_SPHERE));
		//x = 0;
		//tt = 0;
		tt = (tt * 3.1415927) / float(N_SPHERE);
		vec3 A = vec3(3 - 5*I - 5*x, 0.2+1.25*cos(t*2 + 4*tt)*0.5, -5+6*I + 6*x);
		float R = 0.25; // (1.0-I)*0.33 + 0.1;
		S[i]=vec4(A,R);
		
		C[i]=gb; //vec3(0,gb);
		//gb=vec2(0.1,0.1)-gb;
		//gb=vec2(0.1,0.0);

	}
		
	float nearest = 1e10;
	
	vec3 E=V;

	bool isplane_calculated = false;
	
	for (int i=0; i<N_SPHERE; i++)
	{
		vec3 A=S[i].xyz;
		float R=S[i].w;


		float TP=checkPlane(plane, P, V);
		float TS=Q(dot(V,V),2.0*(dot(P,V)-(dot(A,V))),dot(A,A)+dot(P,P)-R*R-(2.0*(dot(A,P))));

		vec3 objColor = planeColor;
		bool planehit = true;
		float T = TP;
		if (TP > 0) {
			if ((TS > 0) && (TS < TP)) {
				T = TS;
				planehit = false;
				objColor = gb;
			}
		} else {
			T = TS;
			planehit = false;
			objColor = gb;
		}		



		if (T > 0.0)
		{			
			if (T < nearest)
			{
				vec3 X=P+T*V;
				vec3 N=planehit?normalize(plane.xyz) : normalize(X-A);
				
				vec3 Ref = reflect(V,N);
				
				float nearestR = 1e10;
				vec3 Rcol=vec3(0.0,0.0,0.0);
				int blocked =0;
// this is the reflection part - no need right now, only the plane is reflecting

				for (int j=0; j<N_SPHERE; j++)
				{
					if (i!=j)
					{
						vec3 A=S[j].xyz;
						float R=S[j].w;
						vec3 V=lightPos;
						vec3 P=X;
						float T=Q(dot(V,V),2.0*(dot(P,V)-(dot(A,V))),dot(A,A)+dot(P,P)-R*R-(2.0*(dot(A,P))));		
						if (T > 0.0)
							blocked=1;
						
						V=normalize(Ref);
						T=Q(dot(V,V),2.0*(dot(P,V)-(dot(A,V))),dot(A,A)+dot(P,P)-R*R-(2.0*(dot(A,P))));		
						if ((T > 0.0) && (planehit))
						{
							if (T<nearestR)
							{
								vec3 X=P+T*V;
								vec3 N=normalize(X-A);
								
								nearestR=T;
								float spec = Lit(N,E,H);
								Rcol=gb*spec + vec3(1,1,1)*pow(spec,3);
								Rcol *= 0.76/(1.0+T*T);
							}
						}
					}
				}
				
			//	vec3 H=normalize(L-V);				
			//	float d = max(dot(N,L),0.0) * 0.5;
			//	float s = pow(max(dot(N,H),0.0),100.0)*.5;
				
				float b = blocked > 0 ? 0.0 : Lit(N,V,H); //d+s;
				nearest = T;

				if (planehit && !isplane_calculated) {
					objColor = mandelbrot(X);
					vec3 fade = normalize(X);
					float fadefac = (fade.z*0.5 + 0.5);

					objColor = objColor*fadefac + vec3(0.1,0.0,0.1)*(1.0 - fadefac);
				}
				
				
				c = objColor*b+vec3(1,1,1)*pow(b,3) + Rcol; // vec3(b,b,b)+b*C[i]; //+Rcol;
			} // if (t<nearest) 
		} // if (t>0.0)	
	}
	
	return c;
}


void main(void)
{
	vec3 rayOrigin, rayDir;
	sampleCamera(vec2(0.0,0.0), rayOrigin, rayDir);


	vec3 H=normalize(lightPos-rayDir);				
	
	vec3 c = vec3(0,0,0);	
		
	c += trace(rayOrigin, rayDir, H);
	gl_FragColor = vec4(c,1.0);
}

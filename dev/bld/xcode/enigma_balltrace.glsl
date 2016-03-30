#version 120

#define PI 3.1415926535898

// TODO: Rewrite this one!!

uniform vec2 iResolution;
uniform float iGlobalTime;
uniform float t_fraczoom;
uniform float t_blend;
uniform vec3 shader_campos;
uniform vec3 shader_camtarget;

vec3 lightPos = normalize(vec3(2,-2,0.5));
vec4 plane = vec4(0,1,0.0,-0.5);

//vec4 plane = vec4(0,0,1,2);

void sampleCamera(vec2 u, out vec3 rayOrigin, out vec3 rayDir)
{
	//vec2 filmUv = (gl_FragCoord.xy + u)/iResolution.xy;
	vec2 filmUv = (gl_FragCoord.xy - iResolution.xy*0.5)/iResolution.y;

	float tx = (2.0*filmUv.x - 1.0)*(iResolution.x/iResolution.y);
	float ty = (1.0 - 2.0*filmUv.y);
	float tz = 0.0;

	// TODO: Create matrix from this

	float FOV = PI/3.; // FOV - Field of view.
	vec3 forward = normalize(shader_camtarget-shader_campos);
    vec3 up = normalize(vec3(0,1,0));
    vec3 right = normalize(cross(forward, up));


    // rd - Ray direction.
    vec3 rd = normalize(forward + FOV*filmUv.x*right + FOV*filmUv.y*up);


	// vec2 sc = vec2(cos(iGlobalTime), sin(iGlobalTime));
	// vec3 rd = normalize(vec3(rayOrigin.x * sc.x - sc.y, rayOrigin.y, sc.x + rayOrigin.x * rayOrigin.y));
	// rayOrigin = rd;

//	rayOrigin = vec3(0.0, 0.0, 2.0);
//	rayDir = normalize(vec3(tx, ty, tz) - rayOrigin);
	rayOrigin = shader_campos;
	lightPos = normalize(shader_campos);
	rayDir = rd;
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

// values and iteration stuff from IQ source (see: shadertoy.com)
vec3 mandelbrot(vec3 planehit) {
	vec2 p = planehit.xz;
	float time = t_fraczoom;

    float zoo = 0.62 + 0.3*cos(0.1*time);
    float coa = cos( 0.15*(1.0-zoo)*time );
    float sia = sin( 0.15*(1.0-zoo)*time );
    zoo = pow( zoo,8.0);
    vec2 xy = vec2( p.x*coa-p.y*sia, p.x*sia+p.y*coa);
    vec2 c = vec2(-.745,.186) + xy*zoo;

    float l = 0.0;
    vec2 z  = vec2(0.0);
    for( int i=0; i<256; i++ )
    {
        // z = z*z + c		
		z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
	
		if( dot(z,z)>(256.0*256.0) ) break;

		l += 1.0;
    }

	// ------------------------------------------------------
    // smooth interation count
	// float sco = co - log2(log(length(z))/log(256.0));

    // equivalent optimized smooth interation count
	float sl = l - log2(log2(dot(z,z))) + 4.0;
    
	// ------------------------------------------------------

    l = sl;

    vec3 col = 0.5 + 0.5*cos( 3.0 + l*0.15 + vec3(0.0,0.6,1.0));
    return col;

}

//
// P - Point, ray origin
// V - Vector, ray direction
// H - lightPos-rayDir

vec3 calcPlane(float TP, vec3 P, vec3 V, vec3 H) {
	vec3 objColor = vec3(0,0,0);
	vec3 N = normalize(plane.xyz);
	vec3 Ref = reflect(V,N);

	vec3 X=P+TP*V;		// point in plane

	//vec3 N=normalize(X-A);
	vec3 PL = normalize(X - H);
	float p_diffuse = dot(PL, N) * 8;

	objColor = mandelbrot(X);
	objColor = objColor*p_diffuse + vec3(0.1,0.0,0.1); // *(1.0 - fadefac);

	return objColor;

}

#define N_SPHERE2	16

vec3 ApplyLight(vec3 P, vec3 X, vec3 A, vec3 V, vec3 objColor) {
	vec3 N=normalize(X-A);	// Normal
	vec3 LV=normalize(P-X);	// Light vector, from campos to intersectionpoint

	// diffuse component
	float diffuse = max(dot(N,LV),0.0);

	// specular component
	//vec3 RV = normalize(reflect(normalize(V), N));
	vec3 RV = normalize(reflect(V, N));
	float specular = pow(max(dot(LV, RV),0.0),8);	// specular 

	// light up sphere
	vec3 c = objColor * diffuse + vec3(1,1,1)*specular;
	return c;

}

vec3 trace2(vec3 P, vec3 V, vec3 H) {
	vec3 c = vec3(0,0,0);
	vec3 objColor = vec3(0,0,0);
	vec3 sphereColor1=vec3(0.1, 0.96, 0.1);
	vec3 sphereColor2=vec3(1.0, 0.96, 0.1);
	vec3 sphereColor3=vec3(0.1, 0.96, 1.0);
	vec3 sphereColor4=vec3(1.0, 0.96, 1.0);

	
	vec4 S[N_SPHERE2];
	vec3 C[N_SPHERE2];


	// // setup up sphere's - in a circle
	// for(int i=0;i<N_SPHERE2;i++) {
	// 	float t = iGlobalTime*2 + i*2.0*PI/N_SPHERE2;	// add iGlobalTime to move
	// 	vec3 A = vec3(2 * sin(t),1, 2 * cos(t));	// position
	// 	float R = 0.5; // (1.0-I)*0.33 + 0.1;
	// 	S[i]=vec4(A,R);	// postion and radius

	// 	C[i]=sphereColor1; //vec3(0,gb);
	// }
	

	float time = iGlobalTime;
	for (int i=0; i<N_SPHERE2; i++)
	{
		float I = float(i)*(1.0/float(N_SPHERE2));
		float t = I*2.0*3.1415927;

		// 'worm' - stop this when blending
		float tt = fract(time);
		float x = fract(time) *(1.0/float(N_SPHERE2));
		tt = (tt * 3.1415927) / float(N_SPHERE2);
		//vec3 A = vec3(3 - 5*I - 5*x, 0.75+1.25*cos(t*2 + 4*tt)*0.5, -5+6*I + 6*x);
		vec3 A = vec3(4 - 8*I - 8*x, 0.5+1.25*cos(t*2 + 4*tt + PI)*0.5, 0);
		// -- end worm
		
		// circle of spheres
		vec3 B = vec3(2 * sin(t+time),1, 2 * cos(t+time));

		// when 'blend' stop motion
		//float blend = 0.5+0.5*sin(iGlobalTime);
		//float blend = 0.0;
		float blend = min(t_blend,1.0);
		A = A*(1.0-blend) + B*blend;


		float R = 0.25; // (1.0-I)*0.33 + 0.1;
		S[i]=vec4(A,R);
		
		C[i]=sphereColor1; //vec3(0,gb);
		//gb=vec2(0.1,0.1)-gb;
		//gb=vec2(0.1,0.0);

	}

	vec3 E=V;

	float nearest = 1e10;
	float TP = checkPlane(plane, P, V);		
	//TP=-1;


	for (int i=0; i<N_SPHERE2; i++)
	{
		vec3 A=S[i].xyz;	// Sphere position
		float R=S[i].w;		// Sphere radius

		// hit 'time' of ray
		float TS=Q(dot(V,V),2.0*(dot(P,V)-(dot(A,V))),dot(A,A)+dot(P,P)-R*R-(2.0*(dot(A,P))));

		bool planehit = true;
		float T = TP;
		
		T = TP;
		if (TP > 0) {
			if ((TS > 0) && (TS < TP)) {
				T = TS;
				planehit = false;
				objColor = C[0];	// Sphere color
			} 
		} else {
			T = TS;
			planehit = false;
			objColor = C[0];	// Sphere color
		}		

		if ((T > 0) && (T < nearest)) {
			if (planehit) {
				c = calcPlane(TP, P, V, H);


				// calculate reflection
				vec3 N = normalize(plane.xyz);
				vec3 Ref = reflect(V,N);
				
				float nearestR = 1e10;
				vec3 Rcol=vec3(0.0,0.0,0.0);
				int blocked =0;

				for(int j=0;j<N_SPHERE2;j++) {
					if (i!=j) {
						vec3 A=S[j].xyz;
						float R=S[j].w;
						vec3 X=P+TP*V;		// point in plane
						vec3 P=X;
						vec3 V=N;

						//vec3 V=lightPos;
						//vec3 P=X;

						float T=Q(dot(V,V),2.0*(dot(P,V)-(dot(A,V))),dot(A,A)+dot(P,P)-R*R-(2.0*(dot(A,P))));		
						if (T > 0.0)
						{
							blocked=1;
							if (T<nearestR)
							{
								vec3 X=P+T*V;

								//Rcol = ApplyLight(P, X, A, V, C[0]);

								vec3 N=normalize(X-A);
								
								nearestR=T;
								float spec = Lit(N,E,H);
								Rcol=C[0]*spec + vec3(1,1,1)*pow(spec,3);
								Rcol *= 0.76/(1.0+T*T);

								nearestR=T;

							}
						}


					}	
				}

				c += Rcol;
				

			} else {		
				vec3 X=P+T*V;		// point on sphere

				c = ApplyLight(P, X, A, V, objColor);

				nearest = T;	
				//objColor.r = T;

				// todo: reflection - copy from 'trace'
			}
		}
	}
	return c;
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
		//vec3 A = vec3(3 - 5*I - 5*x, 0.75+1.25*cos(t*2 + 4*tt)*0.5, -5+6*I + 6*x);
		
		// circle of spheres
		vec3 A = vec3(2 * sin(t),1, 2 * cos(t));


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
	//c += trace(rayOrigin, rayDir, H);		
	c += trace2(rayOrigin, rayDir, lightPos);
	gl_FragColor = vec4(c,1.0);
}

#include <math.h>
#include "vecutil.h"

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif
#define DEG2RAD(x) (((x)*2.0*M_PI)/360.0)
#define RAD2DEG(x) (((x)*360.0)/(2.0*M_PI))
#define EPSILON (.00001)

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif

void linelinedistance(float *v1, float *a1, float *v2, float *a2, float *t1, float *t2)
{
  float a1a1,a2a2,a1a2,a1v1,a1v2,a2v1,a2v2;

  a1a1=vDot(a1,a1);
  a2a2=vDot(a2,a2);
  a1a2=vDot(a1,a2);
  a1v1=vDot(a1,v1);
  a1v2=vDot(a1,v2);
  a2v1=vDot(a2,v1);
  a2v2=vDot(a2,v2);

  *t2 = ( a2v1-a2v2 + a1a2* ((a1v2-a1v1)/a1a1) ) / ( a2a2 - (a1a2*a1a2)/a1a1 );
  *t1 = ( a1v2 + *t2*a1a2 - a1v1 ) / (a1a1);
}

float linepointdistance(float *raypos, float *raydir, float *point)
{
  float tmp1[3];

//  vNorm(raydirnorm,raydir);
  return vDot(raydir,vSub(tmp1,point,raypos))/vDot(raydir,raydir);
}

float rayplaneintersect(float *raypos, float *raydir, float *planep, float *planen)
{
  float pnorm[3],v[3];

  vNorm(pnorm,planen);

  return vDot(pnorm,vSub(v,planep,raypos)) / vDot(pnorm,raydir);
}


float n_raysphereintersect(float *raypos, float *raydir, float *origo, float rad)
{
    float a[3],b,c,t1,t2,tmp1[3],l;
    
//    float raydirnorm[3];
//    vNorm(raydirnorm,raydir);
    
    vSub(a,origo,raypos);
    b=vDot(a,raydir);

    if (b < 0)
        return -1;
    
    c=b*b-vDot(a,a)+rad*rad;
    
    if (c<=0) return -1;
    
    //l=vAbs(raydir);
    
    t1=(b-sqrt(c));

    if (t1 > 0)
        return t1;

    return -1;
/*
    t2=(b+sqrt(c));
    
    if (t1<0 && t2<0) return -1;
    if (t1<0) return t2;
    if (t2<0) return t1;
*/
    
    return min(t1,t2);
}


float raysphereintersect(float *raypos, float *raydir, float *origo, float rad)
{
  float a[3],b,c,t1,t2,tmp1[3],l;

  float raydirnorm[3];
  vNorm(raydirnorm,raydir);

  vSub(a,origo,raypos);
  b=vDot(a,raydirnorm);
  c=b*b-vDot(a,a)+rad*rad;

  if (c<=0) return -1;

  l=vAbs(raydir);

  t1=(b-sqrt(c))/l;
  t2=(b+sqrt(c))/l;

  if (t1<0 && t2<0) return -1;
  if (t1<0) return t2;
  if (t2<0) return t1;

  return min(t1,t2);
}

float raysphereintersectback(float *raypos, float *raydir, float *origo, float rad)
{
  float a[3],b,c,t1,t2,tmp1[3],l;

  float raydirnorm[3];
  vNorm(raydirnorm,raydir);

  vSub(a,origo,raypos);
  b=vDot(a,raydirnorm);
  c=b*b-vDot(a,a)+rad*rad;

  if (c<=0) return -1;

  l=vAbs(raydir);

  t1=(b-sqrt(c))/l;
  t2=(b+sqrt(c))/l;

/*  if (t1<0 && t2<0) return -1;
  if (t1<0) return t2;
  if (t2<0) return t1;*/

  return max(t1,t2);
}


void debugout(char *fmt, ...);

float rotinteract(float *pivot, float *axis, float *pickpoint, float *raypos, float *raydir)
{
  float t,alfa,alfar,f;
  float intersectionp[3],tmp[3],tmp2[3],v1[3],v2[3];
  float projpivot[3],axisnorm[3];

  vNorm(axisnorm,axis);
  
  vSub(projpivot,pivot,vMul(tmp,axisnorm,vDot(axisnorm,vSub(tmp2,pivot,pickpoint))));

  t=rayplaneintersect(raypos,raydir,pickpoint,axis);
  vAdd(intersectionp,raypos,vMul(tmp,raydir,t));

  vNorm(v1,vSub(v1,intersectionp,projpivot));
  vNorm(v2,vSub(v2,pickpoint,projpivot));
  
  f=vDot(v1,v2);
  
  if (f<-1.0 || f>1.0)
	return 0;
  
  alfar=acos(f);
  alfa=RAD2DEG(acos(f));
  
  if (fabs(alfa)<0)
	return 0;

  vCross(tmp,v1,v2);

//  debugout("%f %f %f",t,alfa,vDot(tmp,axis));
  
  if (vDot(tmp,axis)<0)
    return alfa;

  else
    return -alfa;
}

float *projectpointonplane(float *point, float *planep, float *planen, float *pointret)
{
  float nnorm[3],v[3],u[3];

  vNorm(nnorm,planen);

  vSub(pointret,point,vMul(u,nnorm,vDot(vSub(v,point,planep),nnorm)));

  return pointret;
}

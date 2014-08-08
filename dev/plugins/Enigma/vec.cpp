/**
  2002-11-29
    fixed a bug	in qSlerp
**/
#include "vec.h"
#include <math.h>
#include <string.h>

#if !defined(M_PI)
#define M_PI 3.1415926535897932384
#endif

#pragma warning (once : 4244)


#define MAT(m,col,row) ((m)[(row)*4+(col)])
#define sMAT(m,col,row) m[row*4+col]

float *vNeg(float *dest, float *v)
{
    dest[0]=-v[0];
    dest[1]=-v[1];
    dest[2]=-v[2];

  return dest;
}
float *matDup(float *dst, float *src)
{
  memcpy (dst,src,sizeof (float) * 16);
  return dst;
}

float *matMul(float *dest, float *a, float *b)
{
    int i,j; 
    
    for (i=0; i<4; i++)
        for (j=0; j<4; j++) 
            MAT(dest,j,i)=MAT(a,0,i)*MAT(b,j,0)+
            MAT(a,1,i)*MAT(b,j,1)+
            MAT(a,2,i)*MAT(b,j,2)+
            MAT(a,3,i)*MAT(b,j,3);
        
        return dest;
}

float *matIdentity (float *mat)
{
    int i;
    for (i=0;i<4*4;i++) mat[i]=0;
    
    MAT(mat,0,0)=1;
    MAT(mat,1,1)=1;
    MAT(mat,2,2)=1;
    MAT(mat,3,3)=1;
    
    return mat;
}
/*
class function TMatrix.Inverse(inMatrix: TMatrix4x4f): TMatrix4x4f;
var
utMatrix:TMatrix4x4f;
Procedure Swap(var indata:single; var utData:Single);
var
    t:single;
begin
    t:=indata;
    indata:=utdata;
    utdata:=t;
   end;
-----------------------------------------
       var
           i,j,k : Integer;
       max,t,det,pivot : single;

       Begin
           for i:=0 to 3 do
           for j:=0 to 3 do
           if i=j then
               utMatrix.Mat[i,j]:=1
               else
               utMatrix.Mat[i,j]:=0;
           
det:=1;
    for i:=0 to 3 do
    begin
max:=-1;
    for k:=i to 3 do
    if abs(inMatrix.Mat[k,i])>max then
        begin
max:=abs(inMatrix.Mat[k,i]);
j:=k;
  end;
  
    if (max<=0) then
            exit;
        
        if j<>i then
            begin
            for k:=i to 3 do
            swap(inMatrix.Mat[i,k],inMatrix.Mat[j,k]);
            for k:=0 to 3 do
            swap(utMatrix.Mat[i,k],utMatrix.Mat[j,k]);
det:=-det;
    end;
    
pivot:=inMatrix.Mat[i,i];
det:=det*pivot;
    for k:=i+1 to 3 do
    inMatrix.Mat[i,k]:=inMatrix.Mat[i,k]/pivot;
    for k:=0 to 3 do
    utMatrix.Mat[i,k]:=utMatrix.Mat[i,k]/pivot;
    
    for j:=i+1 to 3 do
    begin
t:=inMatrix.Mat[j,i];
    for k:=i+1 to 3 do
        inMatrix.Mat[j,k]:=inMatrix.Mat[j,k]-(inMatrix.Mat[i,k]*t);
                for k:=0 to 3 do
                utMatrix.Mat[j,k]:=utMatrix.Mat[j,k]-(utMatrix.Mat[i,k]*t);
                end;
                
                end;
                
                {---------- backward elimination ----------}
                for i:=3 downto 1 do
                begin
                    for j:=0 to i-1 do
                    begin
t:=inMatrix.Mat[j,i];
                    for k:=0 to 3 do
                    utMatrix.Mat[j,k]:=utMatrix.Mat[j,k]-(utMatrix.Mat[i,k]*t);
                    end;
                    end;
Result:=utMatrix;
       end;
 */
#define mSwap(a,b){ftmp=a;a=b;b=ftmp;}
float *matInv (float *dest, float *in)     
{
    float ftmp;
    float det,max,pivot,t;
    int i,j,k;

    matIdentity(dest);
    det = 1;
    for (i=0;i<4;i++)
    {
        max = -1;
        for (k=i;k<4;k++)
        {
            if (fabs (MAT(in,k,i)) > max)
            {
                max = (float)fabs (MAT(in,k,i));
                j=k;
            }
        }

        if (max<=0)
            break;

        if (j!=i)
        {
            for (k=i;k<4;k++)
                mSwap(sMAT(in,i,k),sMAT(in,j,k));
            for (k=0;k<4;k++)
                mSwap(sMAT(dest,i,k),sMAT(dest,j,k));
            det = -det;
        }

        pivot = MAT(in,i,i);
        det = det*pivot;
        
        for (k=i+1;k<4;k++)
            MAT(in,i,k) = MAT(in,i,k) / pivot;            
        for (k=0;k<4;k++)
            MAT(dest,i,k) = MAT(dest,i,k) / pivot;        
        
        for (j=i+1;j<4;j++)
        {
            t = MAT(in,j,i);
            for (k=i+1;k<4;k++)
                MAT(in,j,k)=MAT(in,j,k) - t * MAT(in,i,k);
            for (k=0;k<4;k++)
                MAT(dest,j,k)=MAT(dest,j,k) - t * MAT(dest,i,k);            
        }        
    }

    // backwards elemination
    for (i=3;i;i--)
    {
        for (j=0;j<i;j++)
        {
            t = MAT(in,j,i);
            for (k=0;k<4;k++)
                MAT(dest,j,k) = MAT(dest,j,k) - t * MAT(dest,i,k);
        }
    }

    return dest;
}
#undef mSwap
float *matInv2(float *dest, float *A)
{
  float B[16];
  int i,k,j;
  float t;

  for (i=0;i<4;i++) for (j=0;j<4;j++) MAT(B,i,j)=0.0;
  MAT(B,0,0) = MAT(B,1,1) = MAT(B,2,2) = MAT(B,3,3) = 1.0; 

  // checke ob diagonalkomponenten alle besetzt
  for (j=0;j<4;j++)
  {
  if (MAT(A,j,j)==0.0) 
  {
    for (k=j+1;k<4;k++) 
    {
      if (MAT(A,k,j)!=0.0)
    {
      // swap row k/j in Matrix A and B !!!
      for (i=0;i<4;i++)
      {
      t = MAT(A,j,i); 
      MAT(A,j,i) = MAT(A,k,i);
      MAT(A,k,i) = (float)t;
      t = MAT(B,j,i); 
      MAT(B,j,i) = MAT(B,k,i);
      MAT(B,k,i) = (float)t;
      }
    }
    break;
      }
  }
  }
  
  // für jede der 4 spalten auf der linken Seite
  for (j=0;j<4;j++)
  {
    // normiere pivot-element j,j
    t=MAT(A,j,j);
  for (k=0;k<4;k++) 
  {  
    MAT(A,j,k)=MAT(A,j,k)/t;MAT(B,j,k)=MAT(B,j,k)/t;
  }
    // eliminiere alle elemente der spalte j, ausser in zeile j
  for (i=0;i<4;i++) if ((i!=j) && MAT(A,i,j) != 0)
  {
    t = MAT(A,i,j);
    for (k=0;k<4;k++)
    {
      MAT(A,i,k) = MAT(A,i,k) - t*MAT(A,j,k);
      MAT(B,i,k) = MAT(B,i,k) - t*MAT(B,j,k);
    }
  }
  }
  memcpy(dest,B,sizeof(float)*16);

  return dest;
}

float *matFromAngleAxis(float *m, float angle, float *axis)
{
    float c1,c2,s;
    float u[3];
    
    c1 = cos(angle);
    c2 = 1.0f - c1;
    s  = sin(angle);
    
    vNorm(u, axis);
    
    m[0] = c1 + c2*u[0]*u[0];
    m[1] =      c2*u[1]*u[0] - s*u[2];
    m[2] =      c2*u[2]*u[0] + s*u[1];
    
    m[4] =      c2*u[0]*u[1] + s*u[2];
    m[5] = c1 + c2*u[1]*u[1];
    m[6] =      c2*u[2]*u[1] - s*u[0];
    
    m[8] =      c2*u[0]*u[2] - s*u[1];
    m[9] =      c2*u[1]*u[2] + s*u[0];
    m[10]= c1 + c2*u[2]*u[2];
    
    m[3]=m[7]=m[11]=0;
    m[12]=m[13]=m[14]=0;
    m[15]=1;
    
    return m;
}

float *vMatIdentity (float *mat)
{
  int i;
  for (i=0;i<4*4;i++) mat[i]=0;

  MAT(mat,0,0)=1;
  MAT(mat,1,1)=1;
  MAT(mat,2,2)=1;
  MAT(mat,3,3)=1;

  return mat;
}
float *vMatGetTrans (float *dst, float *mat)
{
  dst[0] = MAT(mat,0,3);
  dst[1] = MAT(mat,1,3);
  dst[2] = MAT(mat,2,3);
  
  return dst;
}

float *matRotateAngles (float *mat, float x, float y, float z)
{
    float tmp[16],tmp2[16];

    matIdentity(tmp);
    vMatFromAngles(tmp,x,y,z);
    matMul (tmp2,mat,tmp);

    memcpy (mat,tmp2,sizeof (tmp2));
    return mat;
}


float *vMatFromFixedAngles (float *mat, float x, float y, float z)
{
  float a,b;

  a = sin(x)*sin(y);
  b = cos(x)*sin(y);

  
  MAT(mat,0,0) = cos(y)*cos(z);
  MAT(mat,0,1) = cos(y)*sin(z);
  MAT(mat,0,2) =-sin(y);

  MAT(mat,1,0) = a*cos(z) - cos(x)*sin(z);
  MAT(mat,1,1) = a*sin(z) + cos(x)*cos(z);
  MAT(mat,1,2) =   sin(x)*cos(y);

  MAT(mat,2,0) = b*cos(z) + sin(x)*sin(z);
  MAT(mat,2,1) = b*sin(z) - sin(x)*cos(z);
  MAT(mat,2,2) = cos(x)*cos(y);

  return mat;
}


float *vMatFromAngles (float *mat, float x, float y,float z)
{
  float a,b;
  
  a = sin(y)*sin(x);
  b = sin(y)*cos(x);
  
  MAT(mat,0,0) = cos(z)*cos(y);
  MAT(mat,0,1) = sin(z)*cos(x) + a*cos(z);
  MAT(mat,0,2) = sin(z)*sin(x) - b*cos(z);
  
  MAT(mat,1,0) =-sin(z)*cos(y);
  MAT(mat,1,1) = cos(z)*cos(x) - a*sin(z);
  MAT(mat,1,2) = cos(z)*sin(x) + b*sin(z);
  
  MAT(mat,2,0) = sin(y);
  MAT(mat,2,1) =-cos(y)*sin(x);
  MAT(mat,2,2) = cos(y)*cos(x);

  return mat;
  
}
float *vMatTranslate (float *mat, float *v)
{
  MAT(mat,0,3) += v[0];
  MAT(mat,1,3) += v[1];
  MAT(mat,2,3) += v[2];
  
  return mat;
}

float *matTranspose(float *dest, float *mat)
{
  float tmp;
  matIdentity(dest);
  MAT(dest,0,3) -= MAT(mat,0,3);
  MAT(dest,1,3) -= MAT(mat,1,3);
  MAT(dest,2,3) -= MAT(mat,2,3);
/*
  m11 = 1.0f;	m12 = 0.0f;	m13 = 0.0f	;
  m21 = 0.0f;	m22 = 1.0f;	m23 = 0.0f	;
  m31 = 0.0f;	m32 = 0.0f;	m33 = 1.0f	;
*/
  tmp = MAT(mat,0,1); MAT(dest,0,1) = MAT(mat,1,0); MAT(dest,0,1) = tmp;
  tmp = MAT(mat,0,2); MAT(dest,0,2) = MAT(mat,2,0); MAT(dest,0,2) = tmp;
  tmp = MAT(mat,1,2); MAT(dest,1,2) = MAT(mat,2,1); MAT(dest,2,1) = tmp;
  
/*
  transp = m12; m12 = m21; m21 = transp	;
  transp = m13; m13 = m31; m31 = transp	;
  transp = m23; m23 = m32; m32 = transp	;
*/	
  return dest;

  
}

float *vApplyMat(float *dest, float *v, float *mat)
{
  dest[0]=MAT(mat,0,0)*v[0] + MAT(mat,0,1)*v[1] + MAT(mat,0,2)*v[2] + MAT(mat,0,3);
  dest[1]=MAT(mat,1,0)*v[0] + MAT(mat,1,1)*v[1] + MAT(mat,1,2)*v[2] + MAT(mat,1,3);
  dest[2]=MAT(mat,2,0)*v[0] + MAT(mat,2,1)*v[1] + MAT(mat,2,2)*v[2] + MAT(mat,2,3);

  return dest;
}
// just rotation no translation, good for normals
float *vApplyMatRot(float *dest, float *v, float *mat)
{
    dest[0]=MAT(mat,0,0)*v[0] + MAT(mat,0,1)*v[1] + MAT(mat,0,2)*v[2];
    dest[1]=MAT(mat,1,0)*v[0] + MAT(mat,1,1)*v[1] + MAT(mat,1,2)*v[2];
    dest[2]=MAT(mat,2,0)*v[0] + MAT(mat,2,1)*v[1] + MAT(mat,2,2)*v[2];
    
    return dest;
}

float *vAdd(float *dest, float *a, float *b)
{
  dest[0]=a[0]+b[0];
  dest[1]=a[1]+b[1];
  dest[2]=a[2]+b[2];

  return dest;
}

float *vSub(float *dest, float *a, float *b)
{
  dest[0]=a[0]-b[0];
  dest[1]=a[1]-b[1];
  dest[2]=a[2]-b[2];

  return dest;
}

float *vMul(float *dest, float *v, float t)
{
  dest[0]=v[0]*t;
  dest[1]=v[1]*t;
  dest[2]=v[2]*t;

  return dest;
}
float *vScale (float *dest, float *src, float x, float y, float z)
{
  dest[0]=src[0]*x;
  dest[1]=src[1]*y;
  dest[2]=src[2]*z;

  return dest;

}
#define EQ_EPS 0.0001f
int vEquals(float *a, float *b) {
  
  if (fabs(a[0]-b[0]) > EQ_EPS) return 0;
  if (fabs(a[1]-b[1]) > EQ_EPS) return 0;
  if (fabs(a[2]-b[2]) > EQ_EPS) return 0;
  return 1;
}


unsigned long vHash(float *v) {
    unsigned long xi = (unsigned long)(v[0] * 73856093);
    unsigned long yi = (unsigned long)(v[1] * 19349663);
    unsigned long zi = (unsigned long)(v[2] * 83492791);
    unsigned long h = (xi)^(yi)^(zi) ; //same as in paper
    return h;
}


float vAbs(float *v)
{
  return (float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

float *vNorm(float *dest, float *v)
{
  float oneoverl=(float)1.0/vAbs(v);

  dest[0]=oneoverl*v[0];
  dest[1]=oneoverl*v[1];
  dest[2]=oneoverl*v[2];

  return dest;
}

float *vIni(float *dest, float x, float y, float z)
{
  dest[0]=x;
  dest[1]=y;
  dest[2]=z;

  return dest;
}

float *vDup(float *dest, float *src)
{
  dest[0]=src[0];
  dest[1]=src[1];
  dest[2]=src[2];

  return dest;
}

float vDot(float *a, float *b)
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

// x-axis X y-axis = z-axis (left-handed)
float *vCross(float *dest, float *a, float *b)
{
  dest[0] = a[1]*b[2] - a[2]*b[1];
  dest[1] = a[2]*b[0] - a[0]*b[2];
  dest[2] = a[0]*b[1] - a[1]*b[0];

  return dest;
}

int vCmp(float *a, float *b)
{
  return (a[0]==b[0]) && (a[1]==b[1]) && (a[2]==b[2]);
}
float *qIdentity (float *q)
{
  vIni (q,0,0,0);
  q[3] = 1.0f;
  return q;
}
float *qIni (float *q, float x, float y, float z, float w)
{
  q[0] = x;
  q[1] = y;
  q[2] = z;
  q[3] = w;
  return q;
}


float *qDup(float *dst, float *src)
{
  memcpy (dst,src,sizeof (float) * 4);
  return dst;
}

float *qScale (float *qr, float *qa, float scale)
{
  int i;
  for (i=0;i<4;i++)
    qr[i]=qa[i] * scale;

  return qr;
}

float *qInv (float *qr, float *qa )
{
  int i;
  for (i=0;i<3;i++)
    qr[i] = -qa[i];

  qr[3] = qa[3];

  return qr;
}

float qAbs (float *q)
{
  return sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
}

float qDot (float *q1, float *q2)
{
  return q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];

}
float *qNorm(float *dest, float *v)
{
  float oneoverl=(float)1.0/qAbs(v);
  
  dest[0]=oneoverl*v[0];
  dest[1]=oneoverl*v[1];
  dest[2]=oneoverl*v[2];
  dest[3]=oneoverl*v[3];
  
  return dest;
}
float* qAdd(float *q, float *q1, float *q2)
{
  int i;
  for (i=0;i<4;i++)
    q[i] = q1[i]+q2[i];
  
  return q;
}

float *qMul(float *qr, float *qa, float *qb )
{
  float va[3],vb[3],vc[3];
  qr[3] = vDot (qa, qb);
  
  vCross (va, qa, qb);
  vMul (vb, qa, qb[3]);
  vMul (vc, qb, qa[3]);
  vAdd(va,va,vb);
  vAdd(qr, va, vc);
  
  qNorm(qr, qr);
  return qr;
}

///////////////////////////////////////////////////////////////////////////
/**
* Operator to cast a quaternion to a 3x3 matrix.
* Only works with unit quaternions, pay attention to what you do...
I don't normalize quat before proceding for speed reasons.
* \return a 3x3 matrix built from the quaternion
*/
///////////////////////////////////////////////////////////////////////////
/*
float *qToMat4x4 (float *mat, float *q)
{
  float w,px,py,pz;
  float xs,ys,zs,wx,wy,wz,xx,xy,xz,yy,yz,zz;

  qNorm(q,q);
  
  px = q[0];
  py = q[1];
  pz = q[2];
  w  = q[3];
  
   xs = px + px; 
   ys = py + py; 
   zs = pz + pz;
   wx = w * xs; 
   wy = w * ys; 
   wz = w * zs;
   xx = px * xs; 
   xy = px * ys; 
   xz = px * zs;
   yy = py * ys; 
   yz = py * zs; 
   zz = pz * zs;

   
   MAT(mat,0,0) = 1.0f - yy - zz;
   MAT(mat,1,0) = xy - wz;
   MAT(mat,2,0) = xz + wy;
   
   MAT(mat,0,1) = xy + wz;
   MAT(mat,1,1) = 1.0f - xx - zz;
   MAT(mat,2,1) = yz - wx;
   
   MAT(mat,0,2) = xz - wy;
   MAT(mat,1,2) = yz + wx;
   MAT(mat,2,2) = 1.0f - xx - yy;

   mat[3] = mat[7] = 0;
  mat[11] = mat[12] = mat[13] = mat[14] = 0;
  mat[15] = 1;
  
  return mat;
}

*/
float *qToMat4x4 (float *mat, float *qr)
{
    float X,Y,Z,W,xx,xy,xz,xw,yy,yz,yw,zz,zw;
    
    qNorm(qr,qr);
    X = qr[0];
    Y = qr[1];
    Z = qr[2];
    W = qr[3];
    
    xx = X * X;
    xy = X * Y;
    xz = X * Z;
    xw = X * W;
    
    yy = Y * Y;
    yz = Y * Z;
    yw = Y * W;
    
    zz = Z * Z;
    zw = Z * W;

    
    mat[0]  = 1 - 2 * ( yy + zz );
    mat[1]  =     2 * ( xy - zw );
    mat[2]  =     2 * ( xz + yw );
    
    mat[4]  =     2 * ( xy + zw );
    mat[5]  = 1 - 2 * ( xx + zz );
    mat[6]  =     2 * ( yz - xw );
    
    mat[8]  =     2 * ( xz - yw );
    mat[9]  =     2 * ( yz + xw );
    mat[10] = 1 - 2 * ( xx + yy );
    
/*    
    MAT(mat,0,0)  = 1 - 2 * ( yy + zz );
    MAT(mat,1,0)  =     2 * ( xy - zw );
    MAT(mat,2,0)  =     2 * ( xz + yw );
    
    MAT(mat,0,1)  =     2 * ( xy + zw );
    MAT(mat,1,1)  = 1 - 2 * ( xx + zz );
    MAT(mat,2,1)  =     2 * ( yz - xw );
    
    MAT(mat,0,2)  =     2 * ( xz - yw );
    MAT(mat,1,2)  =     2 * ( yz + xw );
    MAT(mat,2,2) = 1 - 2 * ( xx + yy );
    */
    
    mat[3] = mat[7] = 0;
    mat[11] = mat[12] = mat[13] = mat[14] = 0;
    mat[15] = 1;
    
    return mat;
}
float *qToMat3x3 (float *mat, float *qr)
{
  float X,Y,Z,W,xx,xy,xz,xw,yy,yz,yw,zz,zw;
  
  X = qr[0];
  Y = qr[1];
  Z = qr[2];
  W = qr[3];
  
  xx = X * X;
  xy = X * Y;
  xz = X * Z;
  xw = X * W;
  
  yy = Y * Y;
  yz = Y * Z;
  yw = Y * W;
  
  zz = Z * Z;
  zw = Z * W;
  
  MAT(mat,0,0) = 1 - 2 * ( yy + zz );
  MAT(mat,0,1) =     2 * ( xy - zw );
  MAT(mat,0,2) =     2 * ( xz + yw );
  
  MAT(mat,1,0)  =     2 * ( xy + zw );
  MAT(mat,1,1)  = 1 - 2 * ( xx + zz );
  MAT(mat,1,2) =     2 * ( yz - xw );
  
  MAT(mat,2,0)  =     2 * ( xz - yw );
  MAT(mat,2,1)  =     2 * ( yz + xw );
  MAT(mat,2,2) = 1 - 2 * ( xx + yy );
  
  
  return mat;
}



float* qSlerp(float *q, float *q1, float *q2, float t)
{
  float q3[4],q4[4];
  float s1,s2,sa,a,d;

  d = qDot(q1,q2);
  if(d < -1.0f)
    d = -1.0f;
  else
    if(d > 1.0f) d = 1.0f;
    
  a = acos(d);
  sa = sin(a);
  if(sa < 0.0001f)
  {
    int i;
    for (i=0;i<4;i++) q[i] = q1[i];
  }	
  else
  {
    s1 = sin((1.0f-t)*a)/sa;
    s2 = sin(t*a)/sa;
    qScale(q3,q1,s1);
    qScale(q4,q2,s2);	// bug here, fixed by Gnilk 021129
    qAdd(q,q3,q4);
  }
  
  return q;
}

void qToAngleAxis (float *v,float *axis,float *qr)
{
  float ca,sa;

  qNorm(qr,qr);

  ca = acos (qr[3]);
  sa = sin  (ca);
  
  *v = ca * 2;
  
  if ( fabs( sa ) < 0.0005 )
  {
    vIni(axis,0,0,1);
  } else
    {
      vIni(axis,qr[0],qr[1],qr[2]);
      vMul(axis,axis,1.0/sa);
    }

}
float* qFromAngleAxis(float *q, float a, float *v)
{
  float u[3];
  float s;
  
  if(v[0]==0.0f && v[1]==0.0f && v[2]==0.0f)
  {
    q[0] = 1.0f;
    q[1] = 0.0f;
    q[2] = 0.0f;
    q[3] = 0.0f;
    return q;
  }
  
  a *= 0.5f;
  s = (float)sin(a);
  vNorm(u,v);
  
  vMul(q,u,s);
/*
  q[0] = u.x*s;
  q[1] = u.y*s;
  q[2] = u.z*s;
*/
  q[3] = (float)cos(a);
  
  return q;
}
/*
// taken from gamedev.net quat faq...

quaternion_from_euler( QUATERNION *q, VFLOAT ax, VFLOAT ay, VFLOAT az )
  {
    VECTOR3 vx = { 1, 0, 0 }, vy = { 0, 1, 0 }, vz = { 0, 0, 1 };
    QUATERNION qx, qy, qz, qt;

    quaternion_from_axisangle( qx, &vx, rx );
    quaternion_from_axisangle( qy, &vy, ry );
    quaternion_from_axisangle( qz, &vz, rz );

    quaternion_multiply( &qt, &qx, &qy );
    quaternion_multiply( &q,  &qt, &qz );
  }


*/
float *qFromEuler(float *q, float *v)
{
  float vx[]={1,0,0};
  float vy[]={0,1,0};
  float vz[]={0,0,1};
  float qx[4],qy[4],qz[4],qt[4];

  qFromAngleAxis(qx,v[0],vx);
  qFromAngleAxis(qy,v[1],vy);
  qFromAngleAxis(qz,v[2],vz);

  qMul (qt,qx,qy);
  qMul (q,qt,qz);

  return q;
}
float *matGetTranslation(float *mat, float *dest)
{
  dest[0]=mat[12];
  dest[1]=mat[13];
  dest[2]=mat[14];

  return dest;
}

float *matGetXVec(float *mat, float *dest)
{
  dest[0]=mat[0];
  dest[1]=mat[1];
  dest[2]=mat[2];

  return dest;
}

float *matGetYVec(float *mat, float *dest)
{
  dest[0]=mat[4];
  dest[1]=mat[5];
  dest[2]=mat[6];

  return dest;
}

float *matGetZVec(float *mat, float *dest)
{
  dest[0]=mat[8];
  dest[1]=mat[9];
  dest[2]=mat[10];

  return dest;
}

float *matScale(float *mat, float *scale)
{
  float tmp[16],tmp2[16];

  matIdentity(tmp);
  tmp[0] = scale[0];
  tmp[5] = scale[1];
  tmp[10] = scale[2];

  matMul (tmp2,tmp,mat);
  matDup (mat,tmp2);
  return mat;
}

float *matSetTranslation(float *mat, float *dest)
{
    mat[12]=dest[0];
    mat[13]=dest[1];
    mat[14]=dest[2];
    
    return mat;
}
float *matSetXVec(float *mat, float *dest)
{
    mat[0]=dest[0];
    mat[1]=dest[1];
    mat[2]=dest[2];
    
    return mat;
}

float *matSetYVec(float *mat, float *dest)
{
    mat[4]=dest[0];
    mat[5]=dest[1];
    mat[6]=dest[2];
    
    return mat;
}

float *matSetZVec(float *mat, float *dest)
{
    mat[8]=dest[0];
    mat[9]=dest[1];
    mat[10]=dest[2];
    
    return mat;
}


float *matTransposeRot (float *m)
{
    float mtmp[16];
    memcpy (mtmp,m, sizeof (float) * 16);

    m[0] = mtmp[0*4 + 0];
    m[1] = mtmp[1*4 + 0];
    m[2] = mtmp[2*4 + 0];
//    m[3] = mtmp[3*4 + 0];
    
    m[4] = mtmp[0*4 + 1];
    m[5] = mtmp[1*4 + 1];
    m[6] = mtmp[2*4 + 1];
//    m[7] = mtmp[3*4 + 1];
    
    m[8] = mtmp[0*4 + 2];
    m[9] = mtmp[1*4 + 2];
    m[10] = mtmp[2*4 + 2];
//    m[11] = mtmp[3*4 + 2];
    
//    m[12] = mtmp[0*4 + 3];
//    m[13] = mtmp[1*4 + 3];
//    m[14] = mtmp[2*4 + 3];
//    m[15] = mtmp[3*4 + 3];
    
    return m;
       

}
float* matFromVectors(float *m, float *z, float *y)
{
    float right[3],up[3],in[3];
    float lambda;
    int i;
    
    
    vNorm (in,z);
    vNorm (up,y);
    
    /* Make in and up orthogonal by modifying in. */
    lambda = vDot (in,up);
    
    
    for (i=0;i<3;i++)
        up[i] -= lambda*in[i];
    
    vNorm(up,up);
    vCross(right,up,in);
    
    matSetXVec(m,right);
    matSetYVec(m,up);
    matSetZVec(m,in);
    
    
    return m;
}


// this should be the corrsponding lookat function from GL
/*
void GLAPI
gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx,
    GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy,
    GLdouble upz)
{
    int i;
    float forward[3], side[3], up[3];
    GLfloat m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    // Side = forward x up 
    cross(forward, up, side);
    normalize(side);

    // Recompute up as: up = side x forward 
    cross(side, forward, up);

    __gluMakeIdentityf(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslated(-eyex, -eyey, -eyez);
}
*/

// fuckoff looser implemenation stolen from glu...
float *matLookAt (float *m, float *s, float *t, float *up_in)
{
    float forward[3], side[3], up[3];
  float tmp[16];

  vSub (forward,t,s);
  vDup (up,up_in);
    vNorm(forward,forward);

    // Side = forward x up 
  vCross (side,forward,up);
  vNorm (side,side);

    // Recompute up as: up = side x forward 
  vCross (up,side,forward);

    matIdentity(tmp);
  matSetXVec(tmp,side);
  matSetYVec(tmp,up);
  vMul (forward,forward,-1);
  matSetZVec(tmp,forward);

    matSetTranslation(tmp,s);

  matIdentity(m);
  matInv (m,tmp);

  return m;
}

/*
// this is for left-hand coordsys... actually my own, fixed by krikkit... due to left/right fuckups...  
// seems like I never will be able to get that right....
float* matLookAt(float *m, float *s, float *t, float *up)
{
    float m1[16],m2[16],m3[16];
    float in[3];
    float inn[3];
   

    float right[3];
    float up_new[3];
    

    vSub(in,t,s);

    vNorm (inn,in);

    vCross(right,up,inn);
    vCross(up_new,inn,right);

  vNorm (right,right);
    vNorm (up_new,up_new);
    

    vMatIdentity(m3);
    matSetXVec(m3,right);
    matSetYVec(m3,up_new);
    matSetZVec(m3,inn);

    matInv (m,m3);

    matSetTranslation(m3,s);


    return m;
}
*/
float* matLookAtV(float *m, float sx, float sy, float sz, float tx, float ty, float tz, float ux, float uy, float uz)
{
  float sv[3],tv[3],uv[3];
  vIni (sv,sx,sy,sz);
  vIni (tv,tx,ty,tz);
  vIni (uv,ux,uy,uz);
  return matLookAt (m,sv,tv,uv);
}

/*

  M = Mr*Mt
  M^-1 = (Mr*Mt)^-1 = Mt^-1*Mr^-1


  */



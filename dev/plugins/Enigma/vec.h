#ifndef __VEC_H__
#define __VEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
	#define M_PI (3.1415926535897932384626433832795f)
#endif

float *vAdd(float *dest, float *a, float *b);
float *vSub(float *dest, float *a, float *b);
float *vMul(float *dest, float *v, float t);
float *vScale (float *dest, float *src, float x, float y, float z);
float vAbs(float *v);
float *vNorm(float *dest, float *v);
float *vIni(float *dest, float x, float y, float z);
float *vDup(float *dest, float *src);
float vDot(float *a, float *b);
float *vCross(float *dest, float *a, float *b);
int vCmp(float *a, float *b);
float *vNeg(float *dest, float *v);
unsigned long vHash(float *v);
int vEquals(float *a, float *b);


float *matIdentity (float *mat);
float *matInv(float *dest, float *A);
float *matDup(float *dst, float *src);
float *matFromAngleAxis(float *m, float angle, float *axis);
float *matTranspose(float *dest, float *mat);
float *matRotateAngles (float *mat, float x, float y, float z);

float *vApplyMat(float *dest, float *v, float *mat);
float *vApplyMatRot(float *dest, float *v, float *mat);
float *vMatIdentity (float *mat);
float *vMatFromAngles (float *mat, float x, float y,float z);
float *vMatTranslate (float *mat, float *v);
float *vMatGetTrans (float *dst, float *mat);
float *vMatFromAngles (float *mat, float x, float y,float z);
float *vMatFromFixedAngles (float *mat, float x, float y, float z);

float *matGetXVec(float *mat, float *v);
float *matGetYVec(float *mat, float *v);
float *matGetZVec(float *mat, float *v);
float *matSetXVec(float *mat, float *v);
float *matSetYVec(float *mat, float *v);
float *matSetZVec(float *mat, float *v);
float *matGetTranslation(float *mat, float *dest);
float *matSetTranslation(float *mat, float *dest);
float *matScale(float *mat, float *scale);

float *matMul(float *dest, float *a, float *b);
float *matTransposeRot (float *m);
float* matFromVectors(float *m, float *z, float *y);
float* matLookAt(float *m, float *s, float *t, float *up);
float* matLookAtV(float *m, float sx, float sy, float sz, float tx, float ty, float tz, float upx, float upy, float upz);


// quaternions routines...
float *qIdentity (float *q);
float *qIni (float *q, float x, float y, float z, float w);
float *qDup(float *dst, float *src);
float *qScale (float *qr, float *qa, float scale);
float *qInv (float *qr, float *qa );
float qAbs (float *q);
float qDot (float *q1, float *q2);
float *qNorm(float *dest, float *v);
float* qAdd(float *q, float *q1, float *q2);
float *qMul(float *qr, float *qa, float *qb );
float *qToMat4x4 (float *mat, float *qr);
float *qToMat3x3 (float *mat, float *qr);
float* qSlerp(float *q, float *q1, float *q2, float t);
void qToAngleAxis (float *v,float *axis,float *qr);
float* qFromAngleAxis(float *q, float a, float *v);
float *qFromEuler(float *q, float *v);


#ifdef __cplusplus
}
#endif


#endif
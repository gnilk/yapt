#include "glShaderBase.h"
#include "glBillboard.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"


#include <math.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <OpenGl/glu.h>
#endif

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


/// Draw sprites
void OpenGLBillboard::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	generateUV = pInstance->CreateProperty("generateUV", kPropertyType_Bool, "false", "");
	uvScale = pInstance->CreateProperty("uvScale", kPropertyType_Vector, "1,1,1", "");
	pointSize = pInstance->CreateProperty("pointsize", kPropertyType_Float,"1","");


	outVertexCount = pInstance->CreateOutputProperty("outVertexCount", kPropertyType_Integer, "0", "");
	outVertexData = pInstance->CreateOutputProperty("outVertexData", kPropertyType_UserPtr, NULL, "");
	outQuadCount = pInstance->CreateOutputProperty("outQuadCount", kPropertyType_Integer, "0", "");
	outQuadData = pInstance->CreateOutputProperty("outQuadData", kPropertyType_UserPtr, NULL, "");
	outVertexUVData = pInstance->CreateOutputProperty("outVertexUVData", kPropertyType_UserPtr, NULL, "");

}

void OpenGLBillboard::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLBillboard::BeginSimple() {
	float modelview[16];
	int i,j;

	// save the current modelview matrix
	glPushMatrix();

	// get the current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

	// undo all rotations
	// beware all scaling is lost as well 
	for( i=0; i<3; i++ ) 
		for( j=0; j<3; j++ ) {
			if ( i==j )
				modelview[i*4+j] = 1.0;
			else
				modelview[i*4+j] = 0.0;
		}

	// set the modelview with no rotations and scaling
	glLoadMatrixf(modelview);	
}
void OpenGLBillboard::EndSimple() {
	// restores the modelview matrix
	glPopMatrix();
}

void OpenGLBillboard::Render2(double t, IPluginObjectInstance *pInstance) {

	BeginSimple();
	int n = numVertex->v->int_val;
	float *p = (float *)vertexData->v->userdata;
	for(int i=0;i<n;i++) {
		float x = p[i*3+0];
		float y = p[i*3+1];
		float z = p[i*3+2];
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(.1+x, -.1+y, z);
			glVertex3f(.1+x, .1+y, z);
			glVertex3f(-.1+x, .1+y, z);
			glVertex3f(-.1+x, -.1+y, z);
		glEnd();
	}
	//drawObject();
	EndSimple();

}
static float *vd = NULL;

void OpenGLBillboard::Render(double t, IPluginObjectInstance *pInstance) {

	float mat[16];
	float up[3],right[3];
	float A[3],B[3],C[3],D[3];
	float *vs,*pos;
	float *szPtr;
	float size;
	int *quadindex;



	int n = numVertex->v->int_val;
 	vs = (float *)vertexData->v->userdata;
 	vd = (float *)outVertexData->v->userdata;

 	if (outVertexData->v->userdata == NULL) {
 		vd = (float *)malloc(n * sizeof(float)*3*4);
 		outVertexData->v->userdata = vd;
 		outVertexCount->v->int_val = n * 4; 		
 	}

 	quadindex = (int *)outQuadData->v->userdata;
 	if (outQuadData->v->userdata == NULL) {
 		quadindex = (int *)malloc(n * sizeof(int)*4);
 		outQuadData->v->userdata = quadindex;
 		outQuadCount->v->int_val = n;

 	}
 	float uScale = 1.0f;
 	float vScale = 1.0f;
 	bool genuv = false;
	float *outUVData = (float *)outVertexUVData->v->userdata;
 	if (generateUV->v->boolean) {
		if (outUVData == NULL) {
			outUVData = (float *)malloc(n * sizeof(float)*3*4);
			outVertexUVData->v->userdata = outUVData;
		}
		genuv = true;
		uScale = uvScale->v->vector[0];
		vScale = uvScale->v->vector[1];

 	}
 

	glGetFloatv(GL_MODELVIEW_MATRIX , mat);

 	//
 	// vd should be allocated
 	//
    vIni(right,mat[0], mat[4], mat[8]);
    vIni(up,mat[1], mat[5], mat[9]); 	

	vMul (A,right,-1);
	vSub (A,A,up);
	vSub (B,right,up);
	vAdd (C,right,up);
	vAdd (D,vMul (D,right,-1),up);

	int vqs = 3;	// vertex quad size

	size = pointSize->v->float_val;	// could be vector

	for (int i=0;i<n;i++)
	{
		pos = &vs[i * 3]; // source is always a triple float vector
		vIni(&vd[(4*i+0)*vqs],pos[0]+A[0]*size/2,pos[1]+A[1]*size/2,pos[2]+A[2]*size/2);
		vIni(&vd[(4*i+1)*vqs],pos[0]+B[0]*size/2,pos[1]+B[1]*size/2,pos[2]+B[2]*size/2);
		vIni(&vd[(4*i+2)*vqs],pos[0]+C[0]*size/2,pos[1]+C[1]*size/2,pos[2]+C[2]*size/2);
		vIni(&vd[(4*i+3)*vqs],pos[0]+D[0]*size/2,pos[1]+D[1]*size/2,pos[2]+D[2]*size/2);

		quadindex[i*4+0] = i*4+3;
		quadindex[i*4+1] = i*4+2;
		quadindex[i*4+2] = i*4+1;
		quadindex[i*4+3] = i*4+0;

		if (genuv) {
			vIni(&outUVData[(i*4+0)*vqs],0.0*uScale, 0.0*vScale, 0.0);
			vIni(&outUVData[(i*4+1)*vqs],0.0*uScale, 1.0*vScale, 0.0);
			vIni(&outUVData[(i*4+2)*vqs],1.0*uScale, 1.0*vScale, 0.0);
			vIni(&outUVData[(i*4+3)*vqs],1.0*uScale, 0.0*vScale, 0.0);
		}

	}


	// glBegin(GL_QUADS);
	// for(int i=0;i<n;i++) {
	// 	glVertex3fv(&vd[(i*4+0)*vqs]);
	// 	glVertex3fv(&vd[(i*4+1)*vqs]);
	// 	glVertex3fv(&vd[(i*4+2)*vqs]);
	// 	glVertex3fv(&vd[(i*4+3)*vqs]);
	// }
	// glEnd();

	// if (useShaders) {
	// 	OpenGLShaderBase::Detach();
	// }

}


void OpenGLBillboard::PostRender(double t, IPluginObjectInstance *pInstance) {

}

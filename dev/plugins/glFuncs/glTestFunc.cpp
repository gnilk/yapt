//
// test functions for open gl - i.e. old code..
//
#include "glTestFunc.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

void OpenGLRenderContext::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	this->ySys = ySys;
}

void OpenGLRenderContext::Render(double t, IPluginObjectInstance *pInstance) {

	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IOpenGLContextParams *contextParams = (IOpenGLContextParams *)pContext->GetContextParamObject();
	width = contextParams->GetFrameBufferWidth();
   	height = contextParams->GetFrameBufferHeight();

	glViewport(0, 0, width, height);
	// Clear color buffer to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Select and setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0f, (GLfloat) width/ (GLfloat) height, 1.0f, 100.0f);

	// Select and setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 20.0f, 0.0f,    // Eye-position
			0.0f, 0.0f, 0.0f,   // View-point
			0.0f, 0.0f, 1.0f);  // Up-vector

}

void OpenGLRenderContext::PostInitialize(ISystem *ySys,
		IPluginObjectInstance *pInstance) {

}

void OpenGLRenderContext::PostRender(double t,
		IPluginObjectInstance *pInstance) {

	// TODO: Copy to texture here!

}

////////
void OpenGLTriangle::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	numIndex = pInstance->CreateProperty("numindex", kPropertyType_Integer, "0","");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	speed = pInstance->CreateProperty("speed", kPropertyType_Float, "100", "");
}

static void renderBoundingSphere(float *mid, double radius)
{
	glColor3f(1,0.5,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1], mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,1,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double y = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0], mid[1]+y, mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,0.5,1);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double y = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1]+y, mid[2]);
		}
	glEnd();
}	

void OpenGLTriangle::Render(double t, IPluginObjectInstance *pInstance) {
	// float m_speed = speed->v->float_val;
	// glBegin(GL_TRIANGLES);
	// int idx = 0;
	// float *pVtx = (float *) vertexData->v->userdata;
	// int *pIdx = (int *) indexData->v->userdata;

	// glColor3f(1.0f, 0.0f, 0.0f);
	// glVertex3fv(&pVtx[pIdx[0] * 3]);
	// glColor3f(0.0f, 1.0f, 0.0f);
	// glVertex3fv(&pVtx[pIdx[1] * 3]);
	// glColor3f(0.0f, 0.0f, 1.0f);
	// glVertex3fv(&pVtx[pIdx[2] * 3]);

	// glEnd();

	float mid[3] = {0,0,0};
	renderBoundingSphere(mid, 1.0f);


}

void OpenGLTriangle::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLTriangle::PostRender(double t, IPluginObjectInstance *pInstance) {

}

void OpenGLCamera::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	position = pInstance->CreateProperty("position", kPropertyType_Vector, "0,0,1","");
	target = pInstance->CreateProperty("target", kPropertyType_Vector, "0,0,0","");
	upvector = pInstance->CreateProperty("upvector", kPropertyType_Vector, "0,1,0","");
}
void OpenGLCamera::Render(double t, IPluginObjectInstance *pInstance) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(position->v->vector[0],position->v->vector[1],position->v->vector[2],
			target->v->vector[0], target->v->vector[1], target->v->vector[2],
			upvector->v->vector[0],upvector->v->vector[1],upvector->v->vector[2]);
}
void OpenGLCamera::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLCamera::PostRender(double t, IPluginObjectInstance *pInstance) {

}



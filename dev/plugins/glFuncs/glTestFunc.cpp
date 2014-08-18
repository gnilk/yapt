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
	fov = pInstance->CreateProperty("fov", kPropertyType_Float, "65.0", "");

}

void OpenGLRenderContext::Render(double t, IPluginObjectInstance *pInstance) {

	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	width = contextParams->GetFrameBufferWidth();
   	height = contextParams->GetFrameBufferHeight();

	glViewport(0, 0, width, height);
	// Clear color buffer to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Select and setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov->v->float_val, (GLfloat) width/ (GLfloat) height, 1.0f, 100.0f);

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

void OpenGLDrawTriangles::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	indexCount = pInstance->CreateProperty("indexCount", kPropertyType_Integer, "0","");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	speed = pInstance->CreateProperty("speed", kPropertyType_Float, "100", "");
}

void OpenGLDrawTriangles::Render(double t, IPluginObjectInstance *pInstance) {
	float *vtx = (float *)vertexData->v->userdata;
	int *idx = (int *)indexData->v->userdata;
	glBegin(GL_TRIANGLES);
	int ti = 0;
	while (ti < indexCount->v->int_val) {
		glVertex3fv (&vtx[idx[ti+0]*3]);
		glVertex3fv (&vtx[idx[ti+1]*3]);
		glVertex3fv (&vtx[idx[ti+2]*3]);
		ti+=3;
	}
	glEnd();


	float mid[3] = {0,0,0};
	renderBoundingSphere(mid, 1.0f);


}

void OpenGLDrawTriangles::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLDrawTriangles::PostRender(double t, IPluginObjectInstance *pInstance) {

}

/// Draw points
void OpenGLDrawPoints::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
}


void OpenGLDrawPoints::Render(double t, IPluginObjectInstance *pInstance) {


	float *pVertex = (float *) vertexData->v->userdata;
	glColor3f(1,1,1);
	glBegin(GL_POINTS);
	for(int i=0;i<numVertex->v->int_val;i++) {
		glVertex3fv(&pVertex[i*3]);
	}
	glEnd();

}

void OpenGLDrawPoints::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLDrawPoints::PostRender(double t, IPluginObjectInstance *pInstance) {

}

////
void OpenGLDrawLines::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	indexCount = pInstance->CreateProperty("indexCount", kPropertyType_Integer, "0","");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
}

void OpenGLDrawLines::Render(double t, IPluginObjectInstance *pInstance) {
	float *vtx = (float *)vertexData->v->userdata;
	int *idx = (int *)indexData->v->userdata;
	glBegin(GL_LINES);
	int li = 0;
	while (li < indexCount->v->int_val) {
		// ok, this is fake!
		glVertex3fv (&vtx[idx[li+0]*3]);
		glVertex3fv (&vtx[idx[li+1]*3]);
		glVertex3fv (&vtx[idx[li+1]*3]);
		glVertex3fv (&vtx[idx[li+2]*3]);
		glVertex3fv (&vtx[idx[li+2]*3]);
		glVertex3fv (&vtx[idx[li+0]*3]);
		li+=3;
	}
	glEnd();

	float mid[3] = {0,0,0};
	renderBoundingSphere(mid, 1.0f);


}

void OpenGLDrawLines::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLDrawLines::PostRender(double t, IPluginObjectInstance *pInstance) {

}





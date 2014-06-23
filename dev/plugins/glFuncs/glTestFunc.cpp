//
// test functions for open gl - i.e. old code..
//
#include "glTestFunc.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <GL/glfw.h>

using namespace yapt;

void OpenGLRenderContext::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  width = ySys->GetConfigInt(kConfig_ResolutionWidth,1280);  
  height = ySys->GetConfigInt(kConfig_ResolutionWidth,720);  
}
void OpenGLRenderContext::Render(double t, IPluginObjectInstance *pInstance) {

	glViewport(0, 0, width, height);
    printf("HELLO WORLD\n");

	// Clear color buffer to black
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
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
	numIndex = pInstance->CreateProperty("numindex", kPropertyType_Integer, "0",
			"");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr,
			NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr,
			NULL, "");
	speed = pInstance->CreateProperty("speed", kPropertyType_Float, "100", "");
}

void OpenGLTriangle::Render(double t, IPluginObjectInstance *pInstance) {
	glTranslatef(0.0f, 14.0f, 0.0f);
	float m_speed = speed->v->float_val;
	glRotatef((GLfloat) m_speed, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	int idx = 0;
	float *pVtx = (float *) vertexData->v->userdata;
	int *pIdx = (int *) indexData->v->userdata;

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3fv(&pVtx[pIdx[0] * 3]);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3fv(&pVtx[pIdx[1] * 3]);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3fv(&pVtx[pIdx[2] * 3]);

	glEnd();
}

void OpenGLTriangle::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLTriangle::PostRender(double t, IPluginObjectInstance *pInstance) {

}


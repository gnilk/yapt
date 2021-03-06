#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include "glBasicFuncs.h"

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

// GL Camera

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

// GL Transform

void OpenGLTransform::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	position = pInstance->CreateProperty("position", kPropertyType_Vector, "0,0,0","");
	rotation = pInstance->CreateProperty("rotation", kPropertyType_Vector, "0,0,0","");
	scale = pInstance->CreateProperty("scale", kPropertyType_Vector, "1,1,1","");
}
void OpenGLTransform::Render(double t, IPluginObjectInstance *pInstance) {
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(position->v->vector[0],position->v->vector[1],position->v->vector[2]);
	//glRotate3f();
	glRotatef(rotation->v->vector[0],1,0,0);
	glRotatef(rotation->v->vector[1],0,1,0);
	glRotatef(rotation->v->vector[2],0,0,1);

	glScalef(scale->v->vector[0],scale->v->vector[1],scale->v->vector[2]);

}
void OpenGLTransform::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void OpenGLTransform::PostRender(double t, IPluginObjectInstance *pInstance) {

}

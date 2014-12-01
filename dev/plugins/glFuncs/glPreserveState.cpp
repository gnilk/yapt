#include "glPreserveState.h"
#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <math.h>
#include <OpenGl/glu.h>

void OpenGLPreserveState::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLPreserveState::Render(double t, IPluginObjectInstance *pInstance) {
	GLenum err = glGetError();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

}

void OpenGLPreserveState::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLPreserveState::PostRender(double t, IPluginObjectInstance *pInstance) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("ERR: %d\n",err);
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}


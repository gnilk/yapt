#include "glShaderBase.h"
#include "glDrawPoints.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


/// Draw points
void OpenGLDrawPoints::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");

	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
}

void OpenGLDrawPoints::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	std::string vtxshader(vshader->v->string);
	std::string fragshader(fshader->v->string);

	if (!vtxshader.empty() && !fragshader.empty()) {
	  OpenGLShaderBase::Initialize(ySys, 
	    std::string(vshader->v->string),
	    std::string(fshader->v->string));

	  OpenGLShaderBase::ReloadIfNeeded(true);
	  if (program == NULL) {
	    ySys->GetLogger("GLShaderQuad")->Error("Shader program load error");
	    exit(1);
	  }		

	  useShaders = true;
	}
}

void OpenGLDrawPoints::Render(double t, IPluginObjectInstance *pInstance) {

	float *pVertex = (float *) vertexData->v->userdata;
	glColor3f(1,1,1);

	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}

	glBegin(GL_POINTS);
	for(int i=0;i<numVertex->v->int_val;i++) {
		glVertex3fv(&pVertex[i*3]);
	}
	glEnd();

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

}

void OpenGLDrawPoints::PostRender(double t, IPluginObjectInstance *pInstance) {

}

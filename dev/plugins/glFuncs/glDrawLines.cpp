#include "glShaderBase.h"
#include "glDrawLines.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


/// Draw points
void OpenGLDrawLines::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	lineCount = pInstance->CreateProperty("lineCount", kPropertyType_Integer, "0","");
	lineData = pInstance->CreateProperty("lineData", kPropertyType_UserPtr, NULL, "");

	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
}

void OpenGLDrawLines::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

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

void OpenGLDrawLines::Render(double t, IPluginObjectInstance *pInstance) {

	float *pVertex = (float *) vertexData->v->userdata;
	glColor3f(1,1,1);

	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}

	float *vtx = (float *)vertexData->v->userdata;
	int *idx = (int *)lineData->v->userdata;

	glEnable(GL_DEPTH_TEST);

	glBegin(GL_LINES);
	for(int i=0;i<lineCount->v->int_val;i++) {
		glVertex3fv (&vtx[idx[i*2+0]*3]);
		glVertex3fv (&vtx[idx[i*2+1]*3]);
	}
	glEnd();

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

}

void OpenGLDrawLines::PostRender(double t, IPluginObjectInstance *pInstance) {

}

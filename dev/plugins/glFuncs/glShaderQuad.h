#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"


//#include <OpenGl/glu.h>
#include <GL/GLEW.h>


using namespace yapt;


class OpenGLShaderQuad: public PluginObjectImpl, public OpenGLShaderBase {
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
protected:
	void BeginOrtho();
	void EndOrtho();
protected:
	ISystem *ySys;
	int width, height;
	Property *pixelshader_source;
	Property *vertexshader_source;
	Property *depthwrite;

};


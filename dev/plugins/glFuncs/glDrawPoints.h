#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"

using namespace yapt;

class OpenGLDrawPoints: public PluginObjectImpl, public OpenGLShaderBase {
private:
	Property *numVertex;
	Property *vertexData;
	Property *vshader;
	Property *fshader;
	bool useShaders;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

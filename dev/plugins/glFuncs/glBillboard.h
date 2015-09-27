#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"

using namespace yapt;

class OpenGLBillboard: public PluginObjectImpl {
private:
	Property *numVertex;
	Property *vertexData;
	Property *generateUV;
	Property *uvScale;
	Property *pointSize;
	Property *outVertexCount;
	Property *outVertexData;
	Property *outQuadData;
	Property *outQuadCount;
	Property *outVertexUVData;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	virtual void Render2(double t, IPluginObjectInstance *pInstance);
	void BeginSimple();
	void EndSimple();
};

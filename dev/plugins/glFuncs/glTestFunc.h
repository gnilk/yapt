#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"

using namespace yapt;

class OpenGLRenderContext: public PluginObjectImpl {
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
protected:
	int width;
	int height;
	ISystem *ySys;
	Property *fov;
	Property *clear;
};

class OpenGLDrawTriangles: public PluginObjectImpl {
private:
	Property *indexCount;
	Property *indexData;
	Property *vertexData;
	Property *speed;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

class OpenGLRenderToTexture : public PluginObjectImpl {
private:

public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);

};
// class OpenGLDrawLines : public PluginObjectImpl {
// private:
// 	Property *indexCount;
// 	Property *indexData;
// 	Property *vertexData;
// public:
// 	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
// 	virtual void Render(double t, IPluginObjectInstance *pInstance);
// 	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
// 	virtual void PostRender(double t, IPluginObjectInstance *pInstance);

// };


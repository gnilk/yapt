#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"

using namespace yapt;
class TargetContextParamObject : public IRenderContextParams {
private:
	int w, h;
public:
	void SetResolution(int width, int height) {
		this->w = width;
		this->h = height;
	}
    virtual int GetFrameBufferWidth() { return w; }
	virtual int GetFrameBufferHeight() { return h; }
};
class OpenGLRenderTarget: public PluginObjectImpl {
private:
	// input
	Property *textureWidth;
	Property *textureHeight;
	Property *fov;
	// output
	Property *texture;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	unsigned int idFramebuffer, idTexture, idDepthRenderBuffer;
	TargetContextParamObject contextParameters;

};


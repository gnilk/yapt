#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

using namespace yapt;

class PluginObjectImpl: public IPluginObject {
public:
	PluginObjectImpl();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};


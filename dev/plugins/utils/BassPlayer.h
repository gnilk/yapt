#pragma once

#include "yapt/ySystem.h"
#include "bass.h"

using namespace yapt;

class BassPlayer : public IPluginObject {
private:
	HSTREAM chan;
	bool bPlaying;
	float tLast;

public:
	// IPluginObject
	BassPlayer();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};

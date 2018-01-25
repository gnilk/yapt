#include "yapt/ySystem.h"
#include "yapt/logger.h"

using namespace yapt;

class Constant :
	public IPluginObject
{
protected:
	Property *input[5];
	Property *output[5];
private:
	void CopyInputToOutput();
public:
	Constant();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime) {}
};

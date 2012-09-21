#include "yapt/ySystem.h"
#include "yapt/logger.h"

#ifdef WIN32
#include <malloc.h>
#endif

#include <math.h>

#include "LoadFile.h"
#include "WriteLine.h"

using namespace yapt;


extern "C"
{
	int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class UtilsFactory :
	public IPluginObjectFactory
{
public:
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};
static UtilsFactory factory;


IPluginObject *UtilsFactory::CreateObject(ISystem *pSys, const char *identifier)
{
	ILogger *pLogger = pSys->GetLogger("Utils.Factory");//Logger::GetLogger("StdCurve.Factory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier,"utils.LoadFile"))
	{
		pObject = dynamic_cast<IPluginObject *> (new LoadFile());
	}
	if (!strcmp(identifier,"utils.WriteLine"))
	{
		pObject = dynamic_cast<IPluginObject *> (new WriteLine());
	}
  return pObject;
}

int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.LoadFile");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.WriteLine");
	return 0;
}


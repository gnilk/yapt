/*-------------------------------------------------------------------------
File    : $Archive: ConfigObject.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : 


Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 30.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/ySystem.h"
#include "yapt/logger.h"

#ifdef WIN32
#include <malloc.h>
#endif

using namespace yapt;

extern "C"
{
	int CALLCONV InitializeConfigPlugin(ISystem *ySys);
}

ISystem *pSysPtr;

class InternalPluginFactory :
	public IPluginObjectFactory
{
public:	
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *guid_identifier);
};

class ConfigObject :
	public IPluginObject
{
protected:
	// Video properties
	Property *width, *height, *bpp;
	// System properties
public:
	ConfigObject();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};


static InternalPluginFactory config_factory;

ConfigObject::ConfigObject()
{
}
void ConfigObject::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// video
	width = pInstance->CreateProperty("Resolution.Width", kPropertyType_Integer, "640", "Default screen resolution width in pixels");
	height = pInstance->CreateProperty("Resolution.Height", kPropertyType_Integer, "480", "Default screen resolution height in pixels");
	bpp = pInstance->CreateProperty("Resolution.BPP", kPropertyType_Integer, "32", "Default screen pixel resolution in bits");

}
void ConfigObject::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// translate
	ySys->SetConfigValue("Resolution.Width", width->v->int_val);
	ySys->SetConfigValue("Resolution.Height", height->v->int_val);
	ySys->SetConfigValue("Resolution.BPP", bpp->v->int_val);
}

void ConfigObject::Render(double t, IPluginObjectInstance *pInstance)
{
}
void ConfigObject::PostRender(double t, IPluginObjectInstance *pInstance)
{
}

IPluginObject *InternalPluginFactory::CreateObject(ISystem *pSys, const char *identifier)
{
	pSysPtr = pSys;
	ILogger *pLogger = pSys->GetLogger("InternalPluginFactory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier,"System.Config"))
	{
		pObject = dynamic_cast<IPluginObject *> (new ConfigObject());
	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}

int CALLCONV InitializeConfigPlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&config_factory),"name=System.Config");
	return 0;
}

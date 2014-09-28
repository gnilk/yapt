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
	Property *width, *height;
	// System properties
//	Property *fps;
public:
	ConfigObject();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};


static InternalPluginFactory config_factory;

ConfigObject::ConfigObject()
{
}
void ConfigObject::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// video
	width = pInstance->CreateProperty(kConfig_ResolutionWidth, kPropertyType_Integer, "1280", "Default screen resolution width in pixels");
	height = pInstance->CreateProperty(kConfig_ResolutionHeight, kPropertyType_Integer, "720", "Default screen resolution height in pixels");
}
void ConfigObject::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// translate
	ySys->SetConfigValue(kConfig_ResolutionWidth, width->v->int_val);
	ySys->SetConfigValue(kConfig_ResolutionHeight, height->v->int_val);
//	ySys->SetConfigValue("Resolution.BPP", bpp->v->int_val);
}

void ConfigObject::Render(double t, IPluginObjectInstance *pInstance) {
}
void ConfigObject::PostRender(double t, IPluginObjectInstance *pInstance) {
}
void ConfigObject::Signal(int channelId, const char *channelName, int sigval, double sigtime) {
}

IPluginObject *InternalPluginFactory::CreateObject(ISystem *pSys, const char *identifier)
{
	pSysPtr = pSys; // Curve does not have an ISystem ptr  - use a globally shared variable instead

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

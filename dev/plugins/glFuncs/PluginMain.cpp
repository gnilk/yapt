//
// Curve animation handling for Yapt, facade for goat object
//
#include "yapt/ySystem.h"
#include "yapt/logger.h"

#ifdef WIN32
#include <malloc.h>
#endif

using namespace yapt;

extern "C"
{
	int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class Factory :
	public IPluginObjectFactory
{
public:	
	virtual IPluginObject *CreateObject(const char *guid_identifier);
};


static Factory factory;

IPluginObject *Factory::CreateObject(const char *identifier)
{
	ILogger *pLogger = Logger::GetLogger("glFuncs.Factory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", identifier);
	if (!strcmp(identifier,"gl.RenderContext"))
	{
		//		pObject = dynamic_cast<IPluginObject *> (new YaptCurveFacade());
	}
	if (!strcmp(identifier,"gl.Plot"))
	{
//		pObject = dynamic_cast<IPluginObject *> (new YaptCurveFacade());
	}
	if (!strcmp(identifier,"gl.Rotate3f"))
	{
//		pObject = dynamic_cast<IPluginObject *> (new GenericCurveKey());
	}
	if (!strcmp(identifier,"gl.DrawTriangle"))
	{
//		pObject = dynamic_cast<IPluginObject *> (new VectorCurveKey());
	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}

static void perror()
{
}

// This function must be exported from the lib/dll
int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=gl.RenderContext");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=gl.Plot");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=gl.Rotate3f");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=gl.DrawTriangle");
	return 0;
}
/////////////////////
//class OpenGLRenderContext : public IPluginObject
//{
//public:
//	OpenGLRenderContext();
//	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
//	virtual void Render(double t, IPluginObjectInstance *pInstance);
//	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
//	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
//};

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#ifdef WIN32
#include <malloc.h>
#endif

#include <vector>
#include <math.h>


#include "LoadFile.h"
#include "WriteLine.h"
#include "Container.h"

using namespace yapt;


extern "C"
{
	int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class PropertyTest : public IPluginObject {
public:
	PropertyTest();
	virtual ~PropertyTest();
	// IPluginObject
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	std::vector<Property *> properties;
};


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
	if (!strcmp(identifier,"utils.Container"))
	{
		pObject = dynamic_cast<IPluginObject *> (new Container());
	}

	if (!strcmp(identifier,"utils.LoadFile"))
	{
		pObject = dynamic_cast<IPluginObject *> (new LoadFile());
	}
	if (!strcmp(identifier,"utils.WriteLine"))
	{
		pObject = dynamic_cast<IPluginObject *> (new WriteLine());
	}
	if (!strcmp(identifier,"utils.TestProperties")) 
	{
		pObject = dynamic_cast<IPluginObject *> (new PropertyTest());
	}
  return pObject;
}

int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.LoadFile");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.Container");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.WriteLine");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=utils.TestProperties");
	return 0;
}

/// property test

PropertyTest::PropertyTest()
{

}
PropertyTest::~PropertyTest()
{

}
void PropertyTest::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  properties.push_back(pInstance->CreateProperty("color_rgba", kPropertyType_Color, "0,0,0,0", ""));
  properties.push_back(pInstance->CreateProperty("integer", kPropertyType_Integer, "0", ""));
  properties.push_back(pInstance->CreateProperty("integer_tuple", kPropertyType_IntegerTuple, "0,0", ""));
  properties.push_back(pInstance->CreateProperty("float", kPropertyType_Float, "0.0", ""));
  properties.push_back(pInstance->CreateProperty("float_tuple", kPropertyType_FloatTuple, "0.0,0.0", ""));
  properties.push_back(pInstance->CreateProperty("vector", kPropertyType_Vector, "0.0,0.0,0.0", ""));
  properties.push_back(pInstance->CreateProperty("quat", kPropertyType_Quaternion, "0.0,0.0,0.0,0.0", ""));
  properties.push_back(pInstance->CreateProperty("string", kPropertyType_String, "--text--", ""));
  properties.push_back(pInstance->CreateProperty("usrptr", kPropertyType_UserPtr, NULL, ""));
  properties.push_back(pInstance->CreateProperty("enum", kPropertyType_Enum, "val1", "enum={val1, val2, val3, val4}"));
}
  

void PropertyTest::Render(double t, IPluginObjectInstance *pInstance) {
}

void PropertyTest::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void PropertyTest::PostRender(double t, IPluginObjectInstance *pInstance) {

}




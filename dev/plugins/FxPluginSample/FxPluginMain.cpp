#include "ySystem.h"
#include "goat/logger.h"

#ifdef WIN32
#include <malloc.h>
#endif

using namespace yapt;
using namespace Goat;

extern "C"
{
	int CALLCONV yaptInitializePlugin(ISystem *ySys, IPluginObjectInstance *pInstance);
}

class MyEffectFactory :
	public IPluginObjectFactory
{
public:	
	virtual IPluginObject *CreateObject(const char *guid_identifier);
};


class MyBaseEffect :
	public IPluginObject
{
public:
	MyBaseEffect();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
	virtual void Signal(int channelId, const char *channelName, int sigval, double sigtime);
};
class MyEffect :
	public MyBaseEffect
{
protected:
	//	YHANDLE id;
	Property *my_int;
	Property *result;
public:
	MyEffect();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};
class MyEffect2 :
	public MyBaseEffect
{
protected:
	//	YHANDLE id;
	Property *my_int;
public:
	MyEffect2();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};

class MyEffect3 :
	public  MyBaseEffect
{
protected:
	//	YHANDLE id;
	Property *my_int;
public:
	MyEffect3();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};

class MyResource :
	public MyBaseEffect
{
protected:
	//	YHANDLE id;
	Property *output_image;
	Property *filename;
	void *image_data;
public:
	MyResource();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};


static MyEffectFactory factory;

IPluginObject *MyEffectFactory::CreateObject(const char *guid_identifier)
{
	ILogger *pLogger = Logger::GetLogger("Plugin.Factory");
	IPluginObject *pObject = NULL;
	pLogger->Debug("Trying '%s'", guid_identifier);
	if (!strcmp(guid_identifier, "Plugin.MyEffect1"))
	{
		pObject = dynamic_cast<IPluginObject *> (new MyEffect());
	}
	if (!strcmp(guid_identifier, "Plugin.MyEffect2"))
	{
		pObject = dynamic_cast<IPluginObject *> (new MyEffect2());

	}
	if (!strcmp(guid_identifier, "Plugin.MyEffect3"))
	{
		pObject = dynamic_cast<IPluginObject *> (new MyEffect3());

	}
	if (pObject != NULL) 
	{
		pLogger->Debug("Ok");
	}
	else pLogger->Debug("Failed");
	return pObject;
}

// -- Base effect - simple pass through
MyBaseEffect::MyBaseEffect()
{
}
void MyBaseEffect::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::Render(double t, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::PostRender(double t, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::Signal(int channelId, const char *channelName, int sigval, double sigtime) {
	
}

// -- My effect
MyEffect::MyEffect()
{
	ILogger *pLogger = Logger::GetLogger("Effect");
	pLogger->Debug("CTOR");
}

void MyEffect::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Plugin.Effect1");
	pLogger->Debug("Initialize");	

	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "optional_descriptive_xml");
	result = pInstance->CreateOutputProperty("result", kPropertyType_Integer, "result of my_int * 2");

	// Create an integer
	my_int->v->int_val = 16;	
	pLogger->Debug("Setting 'my_int' property with value=%d",my_int->v->int_val);

	result->v->int_val = my_int->v->int_val * 2;


}

void MyEffect::Render(double t, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect");
	result->v->int_val = my_int->v->int_val * 2;	
	pLogger->Debug("Render, my_int=%d",my_int->v->int_val);
	pLogger->Debug("Render, result=my_int * 2 = %d",result->v->int_val);
}

// - MyEffect2
MyEffect2::MyEffect2()
{
	ILogger *pLogger = Logger::GetLogger("Effect2");
	pLogger->Debug("CTOR");
}

void MyEffect2::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect2");
	pLogger->Debug("Initialize");	

	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "optional_descriptive_xml");
	// Create an integer
	my_int->v->int_val = 1;	
	pLogger->Debug("Created 'my_int' property with value=%d",my_int->v->int_val);	
}

void MyEffect2::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	//
	// pInstance->GetContext();
	// pContext->GetDocument();
	//

	IDocument *pDoc = ySys->GetActiveDocument();
	ILogger *pLogger = Logger::GetLogger("Effect2");
	pLogger->Debug("PostInitialized");
	int nChildren = pDoc->GetNumChildren(pInstance, kNodeType_ObjectInstance);
	pLogger->Debug("nChildren %d",nChildren);
	int i;
	pLogger->Enter();
	for (i=0;i<nChildren;i++)
	{
		IPluginObjectInstance *pObject = pDoc->GetChildAt(pInstance, i, kNodeType_ObjectInstance);

		Property *pKeyChild = pObject->GetProperty("my_int");
		if (pKeyChild!=NULL) pLogger->Debug("Child lookup through name ok, v = %d",pKeyChild->v->int_val);
		else pLogger->Debug("Failed to lookup property 'my_int'");

		int j,nProp;
		nProp = pObject->GetNumInputProperties();
		pLogger->Debug("nProperties %d",nProp);
		for(j=0;j<nProp;j++)
		{
			Property * prop = pObject->GetInputPropertyAt(j);
			pLogger->Debug(" %d:%d",j,prop->v->int_val);

		}	
	}
	pLogger->Leave();
}

void MyEffect2::Render(double t, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect2");
	pLogger->Debug("Render, my_int=%d (sourced)",my_int->v->int_val);
}

// - MyEffect3 - fetch properties defined for the children
MyEffect3::MyEffect3()
{
	ILogger *pLogger = Logger::GetLogger("Effect3");
	pLogger->Debug("CTOR");
}

void MyEffect3::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect3");
	pLogger->Debug("Initialize");	

	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "optional_descriptive_xml");

	// Create an integer
	my_int->v->int_val = 1;	
	pLogger->Debug("Created 'my_int' property with value=%d",my_int->v->int_val);
	/*
	ySys->GetNumChildren(this);
	ySys->GetChildAt(this,i);
	ySys->GetNumProperties(this);
	ySys->GetPropertyAt(this,i);

	ySys->GetNumAttribute(this);
	ySys->GetAttribute(this,i);
	*/
}

void MyEffect3::Render(double t, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect3");
	pLogger->Debug("Render, my_int=%d",my_int->v->int_val);


}
MyResource::MyResource()
{
}
void MyResource::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	this->output_image = pInstance->CreateOutputProperty("image", kPropertyType_Integer, "image");	
	this->filename = pInstance->CreateProperty("filename", kPropertyType_String, "filename.jpg");
	//
	// IStreamHandler *pStreamHandler = ySys->GetCurrentStreamHandler();
	// IStream *pStream = ySys->OpenStream(filename->v->string, kStreamMode_Read);
	// Image *pImage = Image::PNGFromStream(pStream);
	// output_image->v->image_handle = GLHelper::UploadImage(pImage);
	// pStream->Close();
	//
}
void MyResource::Render(double t, IPluginObjectInstance *pInstance)
{
}




// This function must be exported from the lib/dll
static void perror()
{
	kErrorClass eClass;
	kError eCode;
	char estring[256];

	GetYaptLastError(&eClass, &eCode);
	GetYaptErrorTranslation(estring,256);
	printf("ERROR (%d:%d); %s\n",eClass,eCode,estring);
	exit(1);
}
static void yFxInitializePlugin(yapt::ISystem *ySys, IPluginObjectInstance *pInstance)
{

	// name=Test.MyEffect;id={FC4F32E9-8164-4c32-BB6E-079DB8CFDFD8};description=this is just a test;
	//	if (!ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),NULL)) perror();
	//	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"@MyResource");
}



int CALLCONV yaptInitializePlugin(ISystem *ySys)
{
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Plugin.MyEffect1");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Plugin.MyEffect2");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Plugin.MyEffect3");
	return 0;
}

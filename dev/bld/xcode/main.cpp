/*
TODO: [- not done, + in progress, ! finished]
---------------------------------------------
	-> SEE doc/todo.txt
*/
#include <iostream>
#include <stdio.h>
#include <stack>
#include <string>
#include <map>

#ifdef WIN32
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#define snprintf _snprintf
#include <windows.h>
#else
#endif

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"	// only needed for plugin class test
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"

using namespace yapt;

#include "noice/io/io.h"
#include "noice/io/ioutils.h"
#include "noice/io/TextReader.h"

using namespace noice::io;

//int main (int argc, char * const argv[]) 
//{
//	//ILogger *pLogger = Logger::GetLogger("main");
//	//pLogger->Debug("YAPT2 - running tests");
//	yapt::ISystem *system = GetYaptSystemInstance();
//	system->SetConfigValue(kConfig_CaseSensitive,false);
//
//	// trigger all errors
//
//	//	TestEnumParser("apa","enum={cpa,apa,bpa}");
//	//	exit(1);
//}

extern "C"
{
	static int CALLCONV yFxInitializePlugin(yapt::ISystem *ySys);
	static int CALLCONV PlugListFunc(IBaseInstance *pBaseInstance);

}

static int TestEnumParser(const char *val, const char *def);
static void TestMoveNodes();
static void TestURLParser();
static void TestExpat(ISystem *pSys,char *sUrl);
static void TestDescriptorParser();
static void TestPluginScanner();
static void TestEnumeration();
static void TestRemoveNodes();
static void TestExport(IDocument *pDoc);
static void TestIODevice();
static void TestTextReader();

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
};
class MyEffect :
	public MyBaseEffect
{
protected:
//	YHANDLE id;
	Property *my_enum;
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
	Property *output_image;
	Property *filename;
	void *image_data;
	ISystem *pSys;
public:
	MyResource();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
};

static MyEffectFactory factory;

IPluginObject *MyEffectFactory::CreateObject(const char *guid_identifier)
{
	ILogger *pLogger = Logger::GetLogger("Factory");
	IPluginObject *pObject = NULL;
	if (!strcmp(guid_identifier, "Test.MyEffect"))
	{
		pLogger->Debug("Creating 'MyEffect'");
		pObject = dynamic_cast<IPluginObject *> (new MyEffect());
	}
	if (!strcmp(guid_identifier, "Test.MyEffect2"))
	{
		pLogger->Debug("Creating 'MyEffect2'");
		pObject = dynamic_cast<IPluginObject *> (new MyEffect2());

	}
	if (!strcmp(guid_identifier, "Test.MyEffect3"))
	{
		pLogger->Debug("Creating 'MyEffect3'");
		pObject = dynamic_cast<IPluginObject *> (new MyEffect3());

	}
	if (!strcmp(guid_identifier, "Test.MyResource"))
	{
		pLogger->Debug("Creating 'MyResource'");
		pObject = dynamic_cast<IPluginObject *> (new MyResource());

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
void MyBaseEffect::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::Render(double t, IPluginObjectInstance *pInstance)
{
}
void MyBaseEffect::PostRender(double t, IPluginObjectInstance *pInstance)
{
}

// -- My effect
MyEffect::MyEffect()
{
	ILogger *pLogger = Logger::GetLogger("Effect");
	pLogger->Debug("CTOR");
}

void MyEffect::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	ILogger *pLogger = Logger::GetLogger("Effect");
	pLogger->Debug("Initialize");	

	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "0","optional_descriptive_xml");
	result = pInstance->CreateOutputProperty("result", kPropertyType_Integer, "0","result of my_int * 2");

	my_enum = pInstance->CreateProperty("my_int", kPropertyType_Enum, "val1","enum={val1,val2,val3,val4}");
	
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
	
	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "0","optional_descriptive_xml");
	// Create an integer
	my_int->v->int_val = 1;	
	pLogger->Debug("Created 'my_int' property with value=%d",my_int->v->int_val);	
}

void MyEffect2::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// Not needed, pInstance has document pointer
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
		//Property *pKeyChild = ySys->GetInputPropertyForObject(pObject, "my_int");
		if (pKeyChild!=NULL) pLogger->Debug("Child lookup through name ok, v = %d",pKeyChild->v->int_val);
		else pLogger->Debug("Failed to lookup property 'my_int'");

		int j,nProp;
		//nProp = ySys->GetNumInputProperties(pObject);
		nProp = pObject->GetNumInputProperties();
		pLogger->Debug("nProperties %d",nProp);
		for(j=0;j<nProp;j++)
		{
			//Property * prop = ySys->GetInputPropertyAt(pObject,j);
			Property *prop = pObject->GetInputPropertyAt(j);
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

	my_int = pInstance->CreateProperty("my_int",kPropertyType_Integer, "0","optional_descriptive_xml");

	// Create an integer
	my_int->v->int_val = 1;	
	pLogger->Debug("Created 'my_int' property with value=%d",my_int->v->int_val);
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
	this->output_image = pInstance->CreateOutputProperty("image", kPropertyType_UserPtr, NULL, "image");	
	this->filename = pInstance->CreateProperty("filename", kPropertyType_String, "image.jpg", "name of image file");
	this->pSys = ySys;

}
// The resource container will have initialized the stream handler in order for 
void MyResource::Render(double t, IPluginObjectInstance *pInstance)
{
	//
	// IStreamHandler *pStreamHandler = ySys->GetCurrentStreamHandler();
	// IStream *pStream = ySys->OpenStream(filename->v->string, kStreamMode_Read);
	// Image *pImage = Image::PNGFromStream(pStream);
	// output_image->v->image_handle = GLHelper::UploadImage(pImage);
	// pStream->Close();
	//
	Logger::GetLogger("MyResource")->Debug("Rendering, loading file from: %s",filename->v->string);

	// --->> TODO: problem
	IResourceContainer *pResources = pSys->GetActiveDocument()->GetResourceContainer();

	IStream *pImageStream = pResources->OpenStream(filename->v->string,0);
	long sz = pImageStream->Size();
	Logger::GetLogger("MyResource")->Debug("FileSize: %d",sz);
	int res=-1;
	char tmp[256];
	do {
		memset(tmp,256,0);
		res = pImageStream->Read(tmp,255);
	} while(!(res<255));
	pImageStream->Close();	
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


// Declared in BasicFileStream
//extern void InitializeIOLibrary(ISystem *ySys);

static int CALLCONV yFxInitializePlugin(yapt::ISystem *ySys)
{
	Logger::GetLogger("InternalPlugins")->Debug("Registering Test plugins");

	// name=Test.MyEffect;id={FC4F32E9-8164-4c32-BB6E-079DB8CFDFD8};description=this is just a test;
//	if (!ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),NULL)) perror();
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Test.MyResource");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Test.MyEffect");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Test.MyEffect2");
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Test.MyEffect3");

	Logger::GetLogger("InternalPlugins")->Debug("Registering IO Library");
	return 0;
}

static void runConsoleTestSuite() {
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("YAPT2 - running tests");
	yapt::ISystem *system = GetYaptSystemInstance();
	
	//TestTextReader();
	//exit(1);
	system->SetConfigValue(kConfig_CaseSensitive,false);
	// trigger all errors
	
	//	TestEnumParser("apa","enum={cpa,apa,bpa}");
	//	exit(1);
	
	//	TestPluginScanner();
	system->ScanForPlugins(".\\", true);
	
	pLogger->Debug("Initializing built in functionality");
	IBaseInstance *pPlugin = system->RegisterAndInitializePlugin(yFxInitializePlugin, "built-in");
	
	
	//	system->SetConfigValue("key1","value1");
	//	system->SetConfigValue("key2","value2");
	//
	//	pLogger->Debug("key1=%s",system->GetConfigValue("key1"));
	//	pLogger->Debug("key2=%s",system->GetConfigValue("key2"));
	//
	//	system->SetConfigValue("key1","value1_2");
	//
	//	pLogger->Debug("key1=%s",system->GetConfigValue("key1"));
	//IStream *pStream = system->CreateStream("file://example1.xml");
	//pStream->Open();
	
	// TODO: The system should load libraries and initialize them on load
	//pLogger->Debug("Initializing Plugins");
	//system->RegisterAndInitializePlugin(yFxInitializePlugin);
	//pLogger->Debug("Initializing IO Library");
	
	pLogger->Debug("Testing Expat XML Parser");
	TestExpat(system, "file://curve_test.xml");
	
	if (system->GetActiveDocument())
	{
		pLogger->Debug("PostInitialize document --- FIX THIS");
		IDocNode *pRoot = system->GetActiveDocument()->GetTree();
		
		system->GetActiveDocumentController()->PostInitializeNode(pRoot);
		//		system->GetActiveDocument()->PostInitializeNode(pRoot);
	}
	
	//	system->GetActiveDocument()->InitializeAllObjects(system);
	pLogger->Debug("Rendering resources");
	system->GetActiveDocumentController()->RenderResources();
	pLogger->Debug("Rendering nodes");
	system->GetActiveDocumentController()->Render(0.0);
	
	//	TestMoveNodes();
	pLogger->Debug("Dumping");
	system->GetActiveDocument()->DumpRenderTree();
	TestRemoveNodes();
	TestEnumeration();
	TestIODevice();
	TestExport(system->GetActiveDocument());
}

int main (int argc, char * const argv[]) 
{
	runConsoleTestSuite();
	return 0;
}

// ok, works -> move to yObjectDef
static void SplitString(std::list<std::string> *tokens, std::string str, char split)
{
	char *pData = (char *)str.c_str();
	char *pStart = pData;

	while (*pData != '\0')
	{
		if (*pData == split)
		{
			*pData = '\0';
			// prevent '' strings coming from empty tokens (;;;;) if split char = ';'
			if (pStart != pData)
			{
				tokens->push_back(std::string(pStart));
			}
			pStart = pData +1;
		}
		pData++;
	}
	if (pStart != pData)
	{
		tokens->push_back(std::string(pStart));
	}
}
static void TestDescriptorParser()
{
	char *desc = ";;;;name=Test.MyEffect;id={FC4F32E9-8164-4c32-BB6E-079DB8CFDFD8};description=this is just a test";
	std::list<std::string> tokens;
	SplitString(&tokens, std::string(desc),';');

	std::list<std::string>::iterator it;
	for(it=tokens.begin();it!=tokens.end();it++)
	{
		std::list<std::string> kvplist;
		std::string pToken = (std::string)(*it);
		SplitString(&kvplist,pToken,'=');

		Logger::GetLogger("main")->Debug("  %s - %s",kvplist.front().c_str(), kvplist.back().c_str());
		
		kvplist.clear();

	}
	tokens.clear();
	Logger::GetLogger("main")->Debug("Desc: %s",desc);

}
static void TestURLParser()
{
	ILogger *pLogger = Logger::GetLogger("main");
	char *sUrl = "file://example2.xml";
	pLogger->Debug("Parsing URL: %s",sUrl);
	URLParser *url = URLParser::ParseURL(sUrl);
	if (url!=NULL)
	{
		pLogger->Debug("  Service: %s",url->GetService());
		pLogger->Debug("  Path...: %s",url->GetPath());

		yapt::ISystem *system = GetYaptSystemInstance();
		IStream *pStream = system->CreateStream(sUrl, 0);
		if (pStream != NULL)
		{
			pStream->Open(kStreamOp_ReadOnly);
			char tmp[1024];
			int n;
			do {
				n = pStream->Read(tmp, 1024);
				fprintf(stdout,"buf: %s",tmp);
			} while(n==1024);
			pStream->Close();
		}

	} else
	{
		pLogger->Debug("Failed!");
	}
}
// - bla,bla
static void TestExpat(ISystem *pSys,char *sUrl)
{
	pSys->LoadNewDocument(sUrl);
}
// Swap Nodes, essentially tests MoveNode since Swap is implemented with Move
static void TestMoveNodes()
{
	ILogger *pLogger = Logger::GetLogger("main");
	yapt::ISystem *system = GetYaptSystemInstance();
	IDocument *pDoc=system->GetActiveDocument();

	pLogger->Debug("Swapping: doc.render.TestSpline - doc.render.TriRotation");
	IBaseInstance *pInstA = pDoc->GetObject("doc.render.TestSpline");
	IBaseInstance *pInstB = pDoc->GetObject("doc.render.TriRotation");

	IDocNode *pNodeA = pDoc->FindNode(pInstA);
	IDocNode *pNodeB = pDoc->FindNode(pInstB);

	if ((pNodeA != NULL) && (pNodeB != NULL))
	{
		pDoc->SwapNodes(pNodeA,pNodeB);
	}
}
static void TestPluginScanner()
{
	ILogger *pLogger = Logger::GetLogger("main");
#ifdef WIN32
	pLogger->Debug("Testing PluginScanner_Win32");
	PluginScanner_Win32 *pScanner = new PluginScanner_Win32();
#else
	pLogger->Debug("Testing PluginScanner_Nix");
	PluginScanner_Nix *pScanner = new PluginScanner_Nix();
	
#endif
	pScanner->ScanForPlugins(yapt::GetYaptSystemInstance(),".\\",true);
}

static int TestEnumParser(const char *val, const char *def)
{
	int res;
	//ILogger *pLogger = Logger::GetLogger("main");
	//PropertyInstance::ParseEnumString(val,def);
	//res = PropertyInstance::ParseEnumString("apa","miff={cpa,apa,bpa}");
	//res = PropertyInstance::ParseEnumString("apa","enum=(cpa,apa,bpa}");
	//res = PropertyInstance::ParseEnumString("apa","enum={cpa,apa,bpa)");
	//// should be ok
	//res = PropertyInstance::ParseEnumString("apa","enum={,,cpa,apa,bpa}");
	//pLogger->Debug("Enum()=%d",res);


	//char tmp[256];char *sRes; 
	//sRes = PropertyInstance::GetEnumFromValue(tmp,256,3,"enum={,,cpa,apa,bpa}");
	//pLogger->Debug("Enum()=%s",sRes);


	return 1;
}
static int CALLCONV PlugListFunc(IBaseInstance *pBaseInstance)
{
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Plugin: %d:%s",pBaseInstance->GetInstanceType(), pBaseInstance->GetAttributeValue("name"));
	return 0;
}
static int IO_CALLCONV DevListFunc(const char *id)
{
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("IO Device: %s",id);
	return 0;
}
static void TestEnumeration()
{
	ILogger *pLogger = Logger::GetLogger("main");
	ISystem *pSys = yapt::GetYaptSystemInstance();
	pLogger->Debug("Listing plugins!");
	pSys->EnumeratePlugins(PlugListFunc);
	pLogger->Debug("Listing plugin objects!");
	pSys->EnumeratePluginObjects(PlugListFunc);
	pLogger->Debug("Listing URL Handlers!");
	pSys->EnumerateURIHandlers(DevListFunc);

}
static void TestRemoveNodes()
{
	ILogger *pLogger = Logger::GetLogger("main");
	ISystem *pSys = yapt::GetYaptSystemInstance();
	pLogger->Debug("Fetching node: '%s'","doc.render.TestSpline.noname2");
	IBaseInstance *pObject = pSys->GetControllerObject("doc.render.TestSpline.noname2");
	if (pObject != NULL)
	{
		pLogger->Debug("Node, ok, removing object!");
		pSys->GetActiveDocument()->RemoveObject(pObject);
		pLogger->Debug("Dumping after remove");
		pSys->GetActiveDocument()->DumpRenderTree();
	} else
	{
		pLogger->Debug("Unable to find node!");
	}
}
static void TestExport(IDocument *pDoc)
{
	//ExportXML *pExport = new ExportXML(pDoc);
	//pExport->ExportToFile("test_export.xml");
	ISystem *pSys = yapt::GetYaptSystemInstance();
	pSys->SaveDocumentAs("file://test_export.xml",pDoc);
}

static void TestIODevice()
{
	ILogger *pLogger = Logger::GetLogger("main");
	ISystem *pSys = yapt::GetYaptSystemInstance();

	if (pSys->GetIODevice("file") != NULL)
	{
#ifdef WIN32
		pSys->GetIODevice("file")->SetParam("relpath","c:\\temp\\");
#else
		pSys->GetIODevice("file")->SetParam("relpath","/tmp/");
#endif
		pSys->GetIODevice("file")->Initialize();
	} else
	{
		pLogger->Error("Unable to find device handler for 'file'");
	}
}

static void TestTextReader()
{
	ILogger *pLogger = Logger::GetLogger("main");
	pLogger->Debug("Testing TextReader");
	yapt::ISystem *system = GetYaptSystemInstance();

	IStream *pStream = system->CreateStream("file://data/lines.txt",0);
	TextReader reader(pStream);
	char tmp[64];
	memset(tmp,0,64);
	while(reader.Read(tmp,63)>0)
	{
		pLogger->Debug("got: %s",tmp);
		memset(tmp,0,64);
	}
	//char *line;
	//while((line = reader.ReadLine()) != NULL)
	//{
	//	pLogger->Debug("L: %s",line);
	//	free(line);
	//}
}
/*-------------------------------------------------------------------------
File    : $Archive: ySystem.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implementation of the ISystem interface and the System class

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
! Could benefit from some refactoring
- Move the document loading out of here
- Add ability to register importers
</pre>


\History
- 17.09.09, FKling, Implementation
- 09.10.09, FKling, Refactor alot of property look routines
- 20.10.09, FKling, Refactored alot regarding rendering control and object instance handling
- 29.10.09, FKling, Temporary document loader
- 10.11.09, FKling, Moved IO Device handling implementation to IO library
---------------------------------------------------------------------------*/

#include <string>
#include <map>
#include <assert.h>

// TODO: Remove
#include "yapt/ExpatXMLImporter.h"
#include "yapt/ExportXML.h"


#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#include "noice/io/io.h"
#include "noice/io/ioclasses.h"	// we create the device directly
#include "noice/io/ioutils.h"
#include "noice/io/TextReader.h"
//#include "yapt/URLParser.h"
using namespace noice::io;


#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;



// -- System

System::System()
{
	pLogger = Logger::GetLogger("System");
	pContext = NULL;
}
System::~System()
{
	// TODO: Dispose all definintions and clean up
}
int System::ReadAndStoreDefaultSettings()
{
	static const char *sConfigFileNameURL = "file://yapt.config";
	noice::io::IStream *pStream = CreateStream(sConfigFileNameURL,0);
	if (pStream != NULL)
	{
		if (pStream->Open(kStreamOp_ReadOnly))
		{
			TextReader reader(pStream);
			char *line;
			while((line = reader.ReadLine()) != NULL)
			{
				// comment
				if (line[0]!='#')
				{
					char *value = strchr(line, '=');
					if (value != NULL)
					{
						*value = '\0';
						value++;
						SetConfigValue(line, value);
						//pLogger->Debug("'%s' = '%s'",line,value);
					}
				}
				free (line);
			}
			reader.Close();
		} else
		{
			pLogger->Error("Unable to open configuration file '%s'",sConfigFileNameURL);
		}
	} else
	{
		pLogger->Error("Unable to create stream for '%s'",sConfigFileNameURL);
	}
	return 0;
}

// -- public
bool System::RegisterObject(IPluginObjectFactory *factory, const char *sDescription)
{
	bool bRes = false;
	//PluginObjectDefinition *pDef =  new PluginObjectDefinition(factory, sDescription);
	PluginObjectDefinition *pDef = PluginObjectDefinition::Create(factory, sDescription);

	if (pDef)
	{
		const char *id = pDef->GetAttributeValue(kDefintion_Name);
		if (definitions.find(id) == definitions.end())
		{
			definitions.insert(PluginNameDefPair(id, dynamic_cast<IPluginObjectDefinition *>(pDef)));
			pLogger->Info("In lib '%s' registered Object '%s'",pLastLoadedContainer->GetAttributeValue("name"), pDef->GetAttributeValue("name"));
			pLastLoadedContainer->AddObjectDefinition(dynamic_cast<IBaseInstance *>(pDef));
			bRes = true;

			// TODO: notify hooks here
		} else
		{
			pLogger->Error("Object '%s' already registered, skipping reregistration (check your identifiers)");
			delete pDef;
		}

		//definitions.push_back(pDef);

	}
	return bRes;
}


IDocument *System::CreateNewDocument(bool bSetActive)
{
	Context *pContext = new Context();	// Create context for document
	IDocument *pDoc = pContext->GetDocument();

	//pContext->SetNamePrefix("weffo");
	this->pContext = dynamic_cast<IContext *>(pContext);
	return dynamic_cast<IDocument *> (pDoc);
}

IDocument *System::GetActiveDocument()
{
	return (pContext != NULL)?pContext->GetDocument():NULL;
}
void System::SetActiveDocument(IDocument *pDocument)
{
	if (pDocument != NULL) 
	{
		IBaseInstance *pBase = dynamic_cast<IBaseInstance *>(pDocument);
		IContext *pContext = pBase->GetContext();
		this->pContext = pContext;
	} else {
		this->pContext = NULL;
	}

}

//
// TODO: Refactor this one, loading should perhaps be in the document itself?
//
IDocument *System::LoadNewDocument(const char *url)
{
	IDocument *pDoc = NULL;
	noice::io::IStream *pStream = CreateStream(url, 0);
	if (pStream != NULL)
	{
		if (pStream->Open(kStreamOp_ReadOnly)) {
			pDoc = CreateNewDocument(true);	// create a new document
			pLogger->Debug("Loading XML from: %s",url);
			ExpatXMLParser *xml = new ExpatXMLParser(dynamic_cast<ISystem *>(this));
			if (xml->ImportFromStream(pStream, false))
			{
				pLogger->Debug("Document loaded ok");
				GetActiveDocument()->DumpRenderTree();
			} else
			{
				char eString[256];
				yapt::GetYaptErrorTranslation(eString, 256);
				pLogger->Error("%s",eString);
				SetActiveDocument(NULL);
				delete pDoc;
				pDoc = NULL;
				
			}			
		} else {
			pLogger->Error("Unable to open stream");
		}
		pStream->Close();
	}
	return pDoc;
}
void System::SaveDocumentAs(const char *url, IDocument *pDoc)
{
	noice::io::IStream *pStream = CreateStream(url, 0);
	if (pStream != NULL)
	{
		ExportXML *pExport = new ExportXML(pDoc);
		if (!pExport->ExportToStream(pStream))
		{
			pLogger->Error("Unable to create stream for URL: %s",url);
		}
	} else
	{
		pLogger->Error("Unable to create stream for URL: %s",url);
	}
}

IContext *System::GetCurrentContext()
{
	return pContext;
}


IDocumentController *System::GetActiveDocumentController()
{
	return (pContext != NULL)?pContext->GetDocumentController():NULL;
}

IPluginObjectDefinition *System::GetObjectDefinition(const char *name)
{
	IPluginObjectDefinition *pRes = NULL;
	if (definitions.find(name) != definitions.end())
	{
		pRes = definitions[name];
	}
	//return definitions[name];
	return pRes;
}

//
// Returns the controlling object from an external object
//
IBaseInstance *System::GetControllerObject(IPluginObject *pObject)
{
	IBaseInstance *pResult = NULL;
	if (pObject != NULL)
	{
		IDocNode *pNode = Lookup::GetNodeForExtInst(pObject);
		if (pNode != NULL)
		{
			pResult = pNode->GetNodeObject();
		}
	}
	return pResult;
}

IBaseInstance *System::GetControllerObject(const char *name)
{
	IBaseInstance *pResult = NULL;

	pResult = Lookup::GetBaseFromString(name);
	if (pResult == NULL)
	{
		// have to search in tree for the data		
	}
	return pResult;
}

// - Wrapped and refactored to IO library
noice::io::IStream *System::CreateStream(const char *uri, unsigned int flags)
{
	return DeviceManager::GetInstance()->CreateStream(uri, flags);
}

noice::io::IStreamDevice *System::GetIODevice(const char *url_identifier)
{
	return DeviceManager::GetInstance()->GetIODevice(url_identifier);
}
void System::SetIODevice(noice::io::IStreamDevice *pDevice, const char *url_identifier)
{
	DeviceManager::GetInstance()->SetIODevice(pDevice, url_identifier);
}
noice::io::IStreamDevice *System::CreateIODevice(const char *url_identifier)
{
	return DeviceManager::GetInstance()->CreateIODevice(url_identifier);
}

bool System::RegisterIODevice(noice::io::IStreamDeviceFactory *pDeviceFactory, const char *url_identifier, const char *initparam, bool bCreate)
{
	return DeviceManager::GetInstance()->RegisterIODevice(pDeviceFactory, url_identifier, initparam, bCreate);
}



// configuration handling
const char *System::GetConfigValue(const char *key)
{
	const char *retval = NULL;
	if (configuration.find(key) != configuration.end())
	{
		retval = configuration[key].c_str();
	}
	return retval;
}

const int System::GetConfigInt(const char *key)
{
	return GetConfigInt(key, -1);
}

const bool System::GetConfigBool(const char *key)
{
	return GetConfigBool(key, false);
}

const char *System::GetConfigValue(const char *key, const char *defValue)
{
	const char *retval = GetConfigValue(key);
	if(retval) return retval;
	else SetConfigValue(key, defValue);	// set value if not present
	return defValue;
}

const bool System::GetConfigBool(const char *key, bool bDefault)
{
	bool bRes = bDefault;
	const char *val = GetConfigValue(key);
	if (val != NULL)
	{

		if (!strcmp(val,"yes")) bRes = true;
		else if (!strcmp(val,"true")) bRes = true;
		else bRes = false;
	}
	return bRes;
}
const int System::GetConfigInt(const char *key, int iDefault)
{
	int iRes = iDefault;
	const char *val = GetConfigValue(key);
	if (val != NULL)
	{
		iRes = atoi(val);
	}
	return iRes;
}


void System::SetConfigValue(const char *key, const char *value)
{
	if (configuration.find(key) != configuration.end())
	{
		configuration[key] = value;
	} else
	{
		configuration.insert(StrStrPair(key,value));
	}
	pLogger->Debug("  '%s' = '%s'", key, value);
}
void System::SetConfigValue(const char *key, const bool value)
{
	SetConfigValue(key,(value)?"yes":"no");
}
void System::SetConfigValue(const char *key, const int value)
{
	char tmp[64];
	snprintf(tmp,64,"%d",value);
	SetConfigValue(key,tmp);
}
// -- plugin functionality
void System::ScanForPlugins(const char *absPath, bool bRecursive)
{
	// TODO: Need to call the platform specific scanner
	IPluginScanner *pScanner = NULL;
#ifdef WIN32
	pScanner = dynamic_cast<IPluginScanner *>(new PluginScanner_Win32());
#else
	pScanner = dynamic_cast<IPluginScanner *>(new PluginScanner_Nix());
#endif
	if (pScanner != NULL)
	{
		pScanner->ScanForPlugins(yapt::GetYaptSystemInstance(),".",true);
	}
}

IBaseInstance *System::RegisterAndInitializePlugin(PFNINITIALIZEPLUGIN pInitializeFunc, const char *name)
{
	PluginContainer *pContainer = new PluginContainer(pInitializeFunc);
	pLastLoadedContainer=pContainer;
	plugins.push_back(pContainer);

	pContainer->AddAttribute("name",name);
	pLogger->Debug("Register ok, initializing plugin");
	pInitializeFunc(dynamic_cast<ISystem *>(this));
	pLogger->Debug("Plugin ok!");
	return dynamic_cast<IBaseInstance *>(pContainer);
}
// -- enumeration functions
void System::EnumeratePlugins(void *pUser, PFNENUMBASEFUNC pEnumFunc)
{
	size_t i;
	for(i=0;i<plugins.size();i++)
	{
		try
		{
			pEnumFunc(pUser, dynamic_cast<IBaseInstance *>(plugins[i]));
		} catch(...)
		{
			// callback exception
			pLogger->Error("Callback during EnumPlugins caused an exception");
		}
	}
}
void System::EnumeratePluginObjects(void *pUser, PFNENUMBASEFUNC pEnumFunc)
{
	PluginNameDefMap::iterator keys;
	for(keys=definitions.begin(); keys != definitions.end(); keys++)
	{
		PluginNameDefPair pair = *(keys);
		try
		{
			pEnumFunc(pUser, dynamic_cast<IBaseInstance *>(pair.second));
		} catch(...)
		{
			// callback exception
			pLogger->Error("Callback during EnumPluginObjects caused an exception");			
		}
	}
}
void System::EnumerateURIHandlers(noice::io::PFNENUMIODEVICE pEnumFunc)
{
	DeviceManager::GetInstance()->EnumerateURIHandlers(pEnumFunc);
}
//
// -- hooking 
//
void System::RegisterDocumentHook(ISystemDocumentHooks *pHook)
{
	GetHooks()->RegisterHooks(pHook);
}

void System::UnregisterDocumentHook(ISystemDocumentHooks *pHook)
{
	GetHooks()->RemoveHooks(pHook);
}

yapt::ILogger *System::GetLogger(const char *loggerName) {
	return Logger::GetLogger(loggerName);
}


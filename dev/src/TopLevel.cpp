/*-------------------------------------------------------------------------
File    : $Archive: yTopLevel.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements some top level functions which are global throughout
	  the system. Can be incorporated into a singleton or the system class
	  currently I am unsure if System should be a singleton or not

	  Errors delivered due to system function calls (fread in Stream) are
	  coarse grained. Will not tell you why something failed, just that it did fail.
	  Errors coming from internal (calling one function with wrong params) will
	  tell you that.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/logger.h"	// ??? - or move to yapt - ??
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;

#ifdef WIN32
#define snprintf _snprintf
#endif

static System *pSystemInstance = NULL;
static kErrorClass glb_ErrClass = kErrorClass_General;
static kError glb_ErrCode = kError_NoError;
// can't be static for some reason, crashes during startup when hookhandler
// assigns logger and the logger does a push back on the Stl list
static SystemHookHandler *hookHandler = NULL;	

// internal top level - not exposed through ySystem.h
SystemHookHandler *yapt::GetHooks()
{
	return hookHandler;
}

extern "C" 
{
int CALLCONV InitializeConfigPlugin(ISystem *ySys);
int CALLCONV InitializeTimelinePlugin(ISystem *ySys);

// top level function
ISystem * CALLCONV yapt::GetYaptSystemInstance()
{
	if (pSystemInstance == NULL)
	{
		Logger::GetLogger("TopLevel")->Debug("Creating System singleton instance");
		pSystemInstance = new System();
		Logger::GetLogger("TopLevel")->Debug("Creating system default hook handler");
		hookHandler = new SystemHookHandler();
		//Logger::GetLogger("TopLevel")->Debug("Initialize IO Library");
		//InitializeIOLibrary(pSystemInstance);
		Logger::GetLogger("TopLevel")->Debug("Reading default settings");
		pSystemInstance->ReadAndStoreDefaultSettings();
		Logger::GetLogger("TopLevel")->Debug("Registering built in plugin types");
		pSystemInstance->RegisterAndInitializePlugin(InitializeConfigPlugin, "System");
		pSystemInstance->RegisterAndInitializePlugin(InitializeTimelinePlugin, "System");
		Logger::GetLogger("TopLevel")->Debug("Ok, everything initialized..");

	}
	// memory allocation failed?
	if (pSystemInstance == NULL)
	{
		perror("new operator, can't allocate memory for system singleton");
		exit(1);
	}
	return dynamic_cast<ISystem *>(pSystemInstance);
}

void CALLCONV yapt::SetYaptLastError(kErrorClass eClass, kError eCode)
{	
	glb_ErrClass = eClass;
	glb_ErrCode = eCode;

	if (hookHandler != NULL)
	{
		hookHandler->OnError(eClass, eCode);	// notify listeners
	}
}

void CALLCONV yapt::GetYaptLastError(kErrorClass *eClass, kError *eCode)
{
	if (eClass != NULL) *eClass = glb_ErrClass;
	if (eCode != NULL) *eCode = glb_ErrCode;
}

// Error code class translation
static char *GetErrClassTranslation(char *dst, int maxlen)
{
	switch(glb_ErrClass)
	{
	case kErrorClass_General :
		snprintf(dst,maxlen,"General");
		break;
	case kErrorClass_Import :
		snprintf(dst, maxlen, "Import");
		break;
	case kErrorClass_ObjectDefinition :
		snprintf(dst, maxlen,"PluginObjectDefinition");
		break;
	case kErrorClass_StreamIO :
		snprintf(dst, maxlen, "StreamIO");
		break;
	default:
		snprintf(dst, maxlen, "Unknown (%d)",glb_ErrClass);
	}
	return dst;
}

// Error code translation, would be better with a data base
static char *GetErrCodeTranslation(char *dst, int maxlen)
{
	switch(glb_ErrCode)
	{
	case kError_NoError :
		snprintf(dst, maxlen, "No Error");
		break;
	case kError_Unknown :
		snprintf(dst, maxlen, "Unknown, not defined, general");
		break;
	case kError_NoDocument:	// No document assigned to system instance
		snprintf(dst, maxlen, "No document assigned");
		break;
	case kError_NoChildOfType:	// Failed to look up node child of specific type
		snprintf(dst, maxlen, "No child of specified type found");
		break;
	case kError_NoInstance:	// Instance not attached to node
		snprintf(dst, maxlen, "No instance found");
		break;
	case kError_URLParseSyntax:
		snprintf(dst, maxlen, "Syntax error in URL");
		break;
	case kError_CreateStreamForURI:
		snprintf(dst, maxlen, "Unable to create stream object");
		break;
	case kError_StreamOpenForURI:
		snprintf(dst, maxlen, "Unable to open stream");
		break;

			// object definition
	case kError_TokenSyntax :	// Descriptor token has a syntax error, normally no '='
		snprintf(dst, maxlen, "Syntax error in descriptor token, missing '=' as separator?");
		break;
	case kError_DescriptorSyntax:	// Descriptor string available, but contains no tokens, syntax error
		snprintf(dst, maxlen, "No descriptors found, syntax error in string, missing ';' as token separator?");
		break;
	case kError_MissingIdentifier:	// the "identifier" token was not found in the descriptor string
		snprintf(dst, maxlen, "Object identifier not found, object not available");
		break;
	case kError_NoFactory:	// Factory was NULL when trying to create descriptor
		snprintf(dst, maxlen, "No factory, object not available");
		break;
	case kError_NoDescriptor:	// Descriptor was NULL
		snprintf(dst, maxlen, "No descriptor, object not available");
		break;
	case kError_FactoryFailed :
		snprintf(dst, maxlen, "Factory failed to create object");
		break;
    case kError_PropertyBindFailed :
        snprintf(dst, maxlen, "Could not bind property");
        break;
		//
	case kError_ObjectNotFound :
		snprintf(dst, maxlen, "Object not found");
		break;

	case kError_StreamNotOpen :
		snprintf(dst, maxlen, "Operation failed, stream not opened, must call Open first");
		break;
	case kError_StreamOpenError :
		snprintf(dst, maxlen, "Can't open stream");
		break;
	case kError_StreamReadError :
		snprintf(dst, maxlen, "Error while reading stream");
		break;
	default :
		snprintf(dst, maxlen, "Unknown (%d)", glb_ErrCode);
	}
	return dst;
}

char * CALLCONV yapt::GetYaptErrorTranslation(char *dst, int maxlen)
{
	char tmpclass[128];
	char tmpcode[128];

	GetErrClassTranslation(tmpclass, 128);
	GetErrCodeTranslation(tmpcode, 128);

	snprintf(dst, maxlen, "%s:%s",tmpclass,tmpcode);
	return dst;
}
}

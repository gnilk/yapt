/*-------------------------------------------------------------------------
File    : $Archive: SystemHookHandler.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : Adaptor for handling external hooks to some document events

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  ! Need recursion protection, can't allow notifications to be sent while doing a notification callback
  - try/catch around the external call
</pre>


\History
- 05.10.09, FKling, Implementation
- 06.10.09, FKling, Recursion protection

---------------------------------------------------------------------------*/
#include <assert.h>
#include <string>
#include <stack>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;



SystemHookHandler::SystemHookHandler()
{
	bInProgress = false;
	pLogger = Logger::GetLogger("SystemHookHandler");
}
SystemHookHandler::~SystemHookHandler()
{
}

bool SystemHookHandler::BeginCall()
{
	bool bRes = false;
	if (!bInProgress)
	{
		bRes = bInProgress = true;		
	} else
	{
		pLogger->Warning("Recursive Hook call detected, not allowed!");
	}
	return bRes;
}
void SystemHookHandler::EndCall()
{
	if (!bInProgress)
	{
	//	pLogger->Debug("EndCall without BeginCall, ignore!");
	}
	bInProgress = false;
}

void SystemHookHandler::RegisterHooks(ISystemDocumentHooks *pHooks)
{
	pLogger->Info("Register Document Hooks: %p",pHooks);
	docHookList.push_back(pHooks);
}
void SystemHookHandler::RemoveHooks(ISystemDocumentHooks *pHooks)
{
	pLogger->Info("[TODO] Remove Document Hooks: %p",pHooks);
	// TODO: Implement
}
void SystemHookHandler::GetAllHooks(std::vector<ISystemDocumentHooks *> &hooklist)
{
	for (size_t i=0;i<this->docHookList.size();i++)
		docHookList.push_back(this->docHookList[i]);
}
void SystemHookHandler::RegisterHooks(ISystemObjectHooks *pHooks)
{
	pLogger->Info("Register Object Hooks: %p",pHooks);
	objHookList.push_back(pHooks);
}
void SystemHookHandler::RemoveHooks(ISystemObjectHooks *pHooks)
{
	pLogger->Info("[TODO] Remove Object Hooks: %p",pHooks);
	// TODO: implement
}
void SystemHookHandler::GetAllHooks(std::vector<ISystemObjectHooks*> &hooklist)
{
	for(size_t i=0;i<objHookList.size();i++)
	{
		hooklist.push_back(objHookList[i]);
	}
}

void SystemHookHandler::BeginDocumentImport()
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->BeginDocumentImport();
		}
		pLogger->Debug("BeginDocumentImport, ok");

	}
	EndCall();
}
void SystemHookHandler::DocumentImportProgress(double progress)
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->DocumentImportProgress(progress);
		}
		pLogger->Debug("DocumentImportProgress, ok");
	}
	EndCall();
}
void SystemHookHandler::EndDocumentImport()
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->EndDocumentImport();
		}
		pLogger->Debug("EndDocumentImport, ok");
	}
	EndCall();
}

// always synchronous
void SystemHookHandler::BeginDocumentRender()
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->BeginDocumentRender();
		}
		pLogger->Debug("BeginDocumentRender, ok");
	}
	EndCall();
}
void SystemHookHandler::EndDocumentRender()
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->EndDocumentRender();
		}
		pLogger->Debug("EndDocumentRender, ok");
	}
	EndCall();
}

// can be async, use URL to track which resource call belongs to
void SystemHookHandler::BeginResourceImport(const char *url)
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->BeginResourceImport(url);
		}
		pLogger->Debug("BeginResourceImport, ok");
	}
	EndCall();
}
void SystemHookHandler::ResourceImportProgress(const char *url, double progress)
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->ResourceImportProgress(url, progress);
		}
		pLogger->Debug("ResourceImportProgress, ok");
	}
	EndCall();
}
void SystemHookHandler::EndResourceImport(const char *url)
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->EndResourceImport(url);
		}
		pLogger->Debug("EndResourceImport, ok");
	}
	EndCall();
}

void SystemHookHandler::OnError(kErrorClass eClass, kError eCode)
{
	if ((docHookList.size() > 0) && (BeginCall()))
	{
		for(size_t i=0;i<docHookList.size();i++)
		{
			docHookList[i]->OnError(eClass,eCode);
		}
		pLogger->Debug("OnError, ok");
	}
	EndCall();
}

// -- Object hooks
void SystemHookHandler::BeginObjectInitialize(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->BeginObjectInitialize(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::EndObjectInitialize(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->EndObjectInitialize(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::BeginObjectPostInitialize(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->BeginObjectPostInitialize(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::EndObjectPostInitialize(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->EndObjectPostInitialize(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::BeginObjectRender(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->BeginObjectRender(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::EndObjectRender(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->EndObjectRender(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::BeginObjectPostRender(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->BeginObjectPostRender(pInstance);
		}
	}
	EndCall();
}
void SystemHookHandler::EndObjectPostRender(IBaseInstance *pInstance)
{
	if ((objHookList.size() > 0) && (BeginCall()))
	{
		for (size_t i=0;i<objHookList.size();i++)
		{
			objHookList[i]->EndObjectPostRender(pInstance);
		}
	}
	EndCall();
}

/*-------------------------------------------------------------------------
File    : $Archive: yObjectDefinition.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Holds the definition of a registered object. Objects should
          be registered as part of the plugin initialization phase.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <list>
#include <string>
#include <assert.h>

#include "yapt/logger.h"

#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


// -- Object definition
PluginObjectDefinition::PluginObjectDefinition() :
	BaseInstance(kInstanceType_ObjectDefinition)
{
	//TODO: assert factory and description
	pLogger = Logger::GetLogger("PluginObjectDefinition");

	this->factory = NULL;
	this->description = NULL;

}
PluginObjectDefinition::~PluginObjectDefinition()
{
	if (description!=NULL) free(description);
	// don't touch the factory!
}

// -- static 
PluginObjectDefinition *PluginObjectDefinition::Create(IPluginObjectFactory *factory, const char *description)
{
	PluginObjectDefinition *pDef = new PluginObjectDefinition();

	if (!pDef->Initialize(factory, description))
	{
		delete pDef;
		pDef = NULL;
	}
	return pDef;
}

bool PluginObjectDefinition::Initialize(IPluginObjectFactory *factory, const char *description)
{
	bool bRes = false;
	if(factory != NULL)
	{
		this->factory = factory;
		if (description != NULL)
		{
			this->description = strdup(description);
			bRes = ParseDescriptionString();
		} else
		{
			SetYaptLastError(kErrorClass_ObjectDefinition, kError_NoDescriptor);
		}
	} else
	{
		SetYaptLastError(kErrorClass_ObjectDefinition, kError_NoFactory);
	}
	return bRes;
}
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

// -- protected helper
bool PluginObjectDefinition::ParseDescriptionString()
{
	// ease up testing
	bool bRes = false;
	if (description[0]=='@')
	{	
		AddAttribute(kDefintion_Name,&description[1]);	
		bRes = true;
	} else
	{
		bool bHaveIdentifier = false;
		// do actual parsing here, split on ';' and than each substring on '='
		std::list<std::string> tokens;
		std::list<std::string>::iterator it;

		bRes = true;
		SplitString(&tokens, std::string(description), ';');
		if (tokens.size() > 0)
		{
			for(it=tokens.begin();it!=tokens.end();it++)
			{
				std::list<std::string> kvplist;
				std::string pToken = (std::string)(*it);
				SplitString(&kvplist,pToken,'=');
				if (kvplist.size() == 2)
				{
					AddAttribute(kvplist.front().c_str(),kvplist.back().c_str());

					if (!strcmp(kvplist.front().c_str(),kDefintion_Name)) 
					{
						bHaveIdentifier= true;
					}

					kvplist.clear();
				} else
				{
					bRes = false;
					SetYaptLastError(kErrorClass_ObjectDefinition, kError_TokenSyntax);
				}
			}
		} else
		{
			bRes = false;
			SetYaptLastError(kErrorClass_ObjectDefinition, kError_DescriptorSyntax);
			pLogger->Error("Object descriptor string contains no descriptors, check your object registration call, object not available");
		}
		tokens.clear();

		// TODO: Verify attributes

		if (!bHaveIdentifier)
		{
			// assign "name" as identifier
			bRes = false;
			SetYaptLastError(kErrorClass_ObjectDefinition, kError_MissingIdentifier);
			pLogger->Warning("Object identification attribute ('%s') not found in description, object not available",kDefintion_Name);
		} 
	}
	return bRes;
}

void PluginObjectDefinition::Dump()
{
	Logger::GetLogger("Dump")->Debug("Definition: %s",description);
}


IPluginObjectFactory *PluginObjectDefinition::GetFactory()
{
	return this->factory;
}
IPluginObjectInstance *PluginObjectDefinition::CreateInstance()
{
	PluginObjectInstance *pInst = new PluginObjectInstance(dynamic_cast<IPluginObjectDefinition  *>(this));
	// TODO: Remove, the object can assign this
	pInst->SetDocument(yapt::GetYaptSystemInstance()->GetActiveDocument());
	pInst->CreateInstance();
	return dynamic_cast<IPluginObjectInstance *>(pInst);
}

IPluginObject *PluginObjectDefinition::CreateExternalInstance()
{
	IPluginObject *result = NULL;
	if(factory != NULL)
	{
		const char *extid  = GetAttributeValue(kDefintion_Name);
		ISystem *pSys = GetYaptSystemInstance();
		result = factory->CreateObject(pSys, extid);
		if (!result)
		{
			yapt::SetYaptLastError(kErrorClass_ObjectDefinition, kError_FactoryFailed);
		}
	}
	return result;
}

const char *PluginObjectDefinition::GetDescription()
{
	return (const char *)this->description;
}


/*-------------------------------------------------------------------------
File    : $Archive: Lookup.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Singleton collection of a few hash table routines used as lookup tables
          reachable from various locations

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Implement clean up routines
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include <map>
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


PropInstMap Lookup::propertyInstanceMap;
PluginNodeMap Lookup::nodehash;
StrBaseMap Lookup::strBaseMap;

// Property to PropertyInstance lookup
void Lookup::RegisterPropInst(Property *prop, IPropertyInstance *pInst)
{
	if (propertyInstanceMap.find(prop) == propertyInstanceMap.end())
	{
		propertyInstanceMap.insert(PropInstPair(prop, pInst));
	}
}
IPropertyInstance *Lookup::GetPropertyInstance(Property *prop)
{
	return propertyInstanceMap[prop];
}

//
// maps external plugin objects to nodes in the tree
//
void Lookup::RegisterNodeExtInst(IDocNode *pNode, IBaseInstance *pInstance)
{
	if (pInstance->GetInstanceType() == kInstanceType_Object)
	{
		PluginObjectInstance *pObjectInstance = dynamic_cast<PluginObjectInstance *>(pInstance);
		if ((pObjectInstance != NULL) && (pObjectInstance->GetExtObject()!=NULL))
		{
			IPluginObject *pExtObject = pObjectInstance->GetExtObject();
			if (nodehash.find(pExtObject) == nodehash.end())
			{
				nodehash.insert(PluginNodePair(pExtObject,pNode));
			} else
			{
				Logger::GetLogger("NodeToPluginObjectLookup")->Debug("External object already in hash");
			}
		} else
		{
			Logger::GetLogger("NodeToPluginObjectLookup")->Debug("ObjectInstance NULL or not ExtObject");
		}
	}
}
//void NodeToPluginObjectLookup::DeregisterNode(IDocNode *pNode)
//{
//	// todo: find and remove node
//}
void Lookup::DeregisterNodeExtInst(IDocNode *pNode)
{
	PluginNodeMap::iterator it;
	for (it=nodehash.begin();it!=nodehash.end();it++)
	{
		PluginNodePair pair = *(it);
		if (pair.second == pNode)
		{
			nodehash.erase(it);
			break;
		}
	}
}

IDocNode *Lookup::GetNodeForExtInst(IPluginObject *pObject)
{
	IDocNode *pNode = NULL;
	if (nodehash.find(pObject) != nodehash.end())
	{
		pNode=nodehash[pObject];
	}
	return pNode;


}

void Lookup::RegisterStrBase(std::string fqname, IBaseInstance *pInst)
{
	if (strBaseMap.find(fqname) == strBaseMap.end())
	{
		strBaseMap.insert(StrBasePair(fqname,pInst));
	}
}
void Lookup::DeregisterStrBase(std::string fqname)
{
	StrBaseMap::iterator it;
	for (it=strBaseMap.begin();it!=strBaseMap.end();it++)
	{
		StrBasePair pair = *(it);
		//if (pair.first == fqname)
		//{
		//	strBaseMap.erase(it);
		//	break;
		//}
	}
}
IBaseInstance *Lookup::GetBaseFromString(std::string fqname)
{
	IBaseInstance *pBase= NULL;
	if (strBaseMap.find(fqname) != strBaseMap.end())
	{
		pBase=strBaseMap[fqname];
	}
	return pBase;
}

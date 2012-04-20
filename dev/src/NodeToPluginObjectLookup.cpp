// TODO: Move this to "Lookup"
#include "yapt/logger.h"
#include "yapt/yDocument.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

using namespace yapt;


// Register the node IF the node is of right type and the Node is mapped to an object
void NodeToPluginObjectLookup::RegisterNode(IDocNode *pNode, IBaseInstance *pInstance)
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
void NodeToPluginObjectLookup::DeregisterNode(IDocNode *pNode)
{
	// todo: find and remove node
}
IDocNode *NodeToPluginObjectLookup::FindNode(IPluginObject *pObject)
{
	IDocNode *pNode = NULL;
	if (nodehash.find(pObject) != nodehash.end())
	{
		pNode=nodehash[pObject];
	}
	return pNode;


}

/*-------------------------------------------------------------------------
File    : $Archive: yDocument.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : The document holds the rendering tree and supporting functions to work
          with the tree. Each node that goes into the tree is given a specific name
	  this name can be used in order to lookup document nodes. The mapping
	  is kept in the Lookup singleton.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  ! Move Render/PostInitialize/Initialize to an own "DocumentController" object, don't mix control and data management
</pre>


\History
- 17.09.09, FKling, Implementation
- 25.09.09, FKling, Removed recursiveness during Initialize - PostInitialize is bottom-up
- 08.10.09, FKling, Refactored all (almost) execution logic to "DocumentController" 
- 08.10.09, FKling, Moved DocNode to own file.
---------------------------------------------------------------------------*/
#include <assert.h>
#include <string>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#include "noice/io/io.h"

using namespace noice::io;

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;


#define DOC_ROOT ("Document.DocumentRoot")

Document::Document(IContext *pContext) :
	BaseInstance(kInstanceType_Document)
{
	pLogger = Logger::GetLogger("Document");

	SetContext(pContext);	// set the context in the base class 

	this->pDocumentController = NULL;
	std::string fqName; 

	// Set up root node, the document is always the root node of the tree
	root = dynamic_cast<IDocNode *>(new DocNode(dynamic_cast<IDocument *>(this)));
	root->SetNodeObject(dynamic_cast<IBaseInstance *>(this), kNodeType_Document);
	AddAttribute("name","doc");
	fqName = BuildQualifiedName(root);
	SetFullyQualifiedName(fqName.c_str());
	treemap.insert(BaseNodePair(dynamic_cast<IBaseInstance *>(this),root));

	// setup default objects
	// currently we setup a resource container and a render node
	// TODO: Remove resource container when supported from the outside
	resources = new ResourceContainer();//new DocNode(dynamic_cast<IDocument *> (this));
	resources->SetContext(pContext);
	resources->AddAttribute("name","resources");
	AddObjectToTree(dynamic_cast<IBaseInstance *>(this), resources, kNodeType_ResourceContainer);

	BaseInstance *pDummy = new BaseInstance(kInstanceType_RenderNode);
	pDummy->SetContext(pContext);
	pDummy->AddAttribute("name","render");
	renderRoot = AddObjectToTree(dynamic_cast<IBaseInstance *>(this), pDummy, kNodeType_RenderNode);


//	renderVars = new RenderVars();

}
Document::~Document()
{
	
}

void Document::SetDocumentController(IDocumentController *pDocumentController)
{
	this->pDocumentController = pDocumentController;
}

void Document::Write(noice::io::IStream *stream)
{
}

void Document::Read(noice::io::IStream *stream)
{
}

IDocNode *Document::GetTree()
{
	return root;
}

IDocNode *Document::GetRenderTree()
{
	return dynamic_cast<IDocNode *> (renderRoot);
}

IBaseInstance *Document::GetRenderRoot()
{
	return (IBaseInstance *)(renderRoot->GetNodeObject());
}

IResourceContainer *Document::GetResourceContainer()
{
	return dynamic_cast<IResourceContainer *>(resources);
}
IBaseInstance *Document::GetObject(const char *fullQualifiedName)
{
	// TODO: Need lookup str::BasInstance	
	return Lookup::GetBaseFromString(fullQualifiedName);
}

void Document::SwapNodes(IDocNode *pNodeA, IDocNode *pNodeB)
{
	IDocNode *pParentB = pNodeB->GetParent();	// will change after first move..
	MoveNode(pNodeA->GetParent(),pNodeB);
	MoveNode(pParentB, pNodeA);
}


void Document::MoveNode(IDocNode *pINewParent, IDocNode *pINode)
{
	DocNode *pNode = dynamic_cast<DocNode *>(pINode);
	DocNode *pNewParent = dynamic_cast<DocNode *>(pINewParent);

	assert(pNode);
	assert(pNewParent);

	DocNode *pOldParent = dynamic_cast<DocNode *>(pNode->GetParent());
	pOldParent->RemoveChild(pINode);
	pNewParent->AddChild(pINode);
}

void Document::RegisterNode(IDocNode *pNode, IBaseInstance *pObject)
{
	Lookup::RegisterNodeExtInst(pNode, pObject);
}

void Document::DeregisterNode(IDocNode *pNode)
{
	// should remove node from Lookup
	Lookup::DeregisterNodeExtInst(pNode);
}

// looks in hash for node
IDocNode *Document::FindNode(IBaseInstance *pObject)
{
	if (treemap.find(pObject)!=treemap.end())
	{
		return treemap[pObject];
	} else
	{
		pLogger->Debug("Failed to find node for object=%p",pObject);
	}
	return NULL;
}

// Better helper, works with root tree
IDocNode *Document::AddNode(IDocNode *parent, IBaseInstance *pObject, kNodeType type)
{
	IDocNode *pNode = new DocNode(dynamic_cast<IDocument *>(this));
	pNode->SetNodeObject(pObject, type);
	parent->AddChild(pNode);

	treemap.insert(BaseNodePair(pObject,pNode));
	RegisterNode(pNode, pObject);
	
	pLogger->Debug("AddNode, node=%p for object=%p",pNode,pObject);

	return pNode;
}
// Assigns a fully qualified name to the object in the document
std::string Document::BuildQualifiedName(IDocNode *pNode)
{
	std::string name = yapt::GetYaptSystemInstance()->GetConfigValue("Document.DefaultNodeName","noname");
	IBaseInstance *pInst = dynamic_cast<IBaseInstance *>((PluginObjectInstance *)pNode->GetNodeObject());
	if (pInst != NULL)
	{
		const char *objName = pInst->GetAttributeValue("name");
		if (objName != NULL) name = std::string(objName);

	} else
	{
		name = yapt::GetYaptSystemInstance()->GetConfigValue("Document.DefaultRenderName","render");
	}
	if (pNode->GetParent() != NULL)
	{
		name = BuildQualifiedName(pNode->GetParent()).append(".").append(name);
	}
	return name;
}
// Adds any BaseInstance object to tree including building the name
IDocNode *Document::AddObjectToTree(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType)
{
	IDocNode *parentNode = NULL;
	IDocNode *pNode = NULL;

	// Ensure we have an attribute "name"
	if (!object->GetAttribute("name"))
	{
		const char *noname = yapt::GetYaptSystemInstance()->GetConfigValue("Document.DefaultNodeName","noname");
		object->AddAttribute("name",noname);
	}

	parentNode = FindNode(parent);
	if (parentNode != NULL)
	{
		pNode = AddNode(parentNode, object, nodeType);

		BaseInstance *pBase = dynamic_cast<BaseInstance *>(object);
		std::string qName = BuildQualifiedName(pNode);

		// object with this name already registered?
		// append a number to the name until it gets unique
		if (Lookup::GetBaseFromString(qName) != NULL)
		{
			int extCount = 1;
			// Can't reuse the const char * return value since if we update the
			// attribute in the while loop it will be deallocated
			std::string baseName(pBase->GetAttributeValue("name"));

			// will update the name with a numbered version
			while (Lookup::GetBaseFromString(qName) != NULL)
			{				
				char tmp[32];

				std::string curName(baseName);
				snprintf(tmp, 32, "%d", extCount);
				curName.append(tmp);		// itoa not supported..
				pBase->AddAttribute("name",curName.c_str());
				qName = BuildQualifiedName(pNode);

				extCount++;
			}
		}

		pBase->SetFullyQualifiedName(qName.c_str());

		// add a lookup for this one..
		Lookup::RegisterStrBase(qName, object);
	}

	return pNode;
}

IDocNode *Document::AddObject(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType)
{
	IDocNode *pNode = AddObjectToTree(parent,object, nodeType);
	pDocumentController->InitializeNode(pNode);
	return pNode;
}

IDocNode *Document::AddRenderObject(IBaseInstance *parent, IBaseInstance *object)
{
	IDocNode *pNode = AddObjectToTree(parent,object,kNodeType_ObjectInstance);
	pDocumentController->InitializeNode(pNode);
	return pNode;
}

void Document::AddResourceObject(IBaseInstance *parent, IBaseInstance *object)
{
	IDocNode *pNode = AddObjectToTree(parent,object,kNodeType_ObjectInstance);
	IPluginObjectInstance *pInst = dynamic_cast<IPluginObjectInstance *>(object);
	resources->AddResourceInstance(pInst);
	pDocumentController->InitializeNode(pNode);
}
IDocNode *Document::AddMetaObject(IBaseInstance *parent) 
{
	IBaseInstance *pMeta = new BaseInstance(kInstanceType_MetaNode);
	IDocNode *pNode = AddObjectToTree(parent, pMeta, kNodeType_Meta);
	pDocumentController->InitializeNode(pNode);
	return pNode;
}

//
// Removes a node and tracks all objects
//
bool Document::RemoveNodeTraceObjects(IDocNode *pNode, std::vector<IBaseInstance *> &nodeObjects)
{
	bool bRes = true;
	assert(pNode->GetParent());
	int n,i;
	n =  pNode->GetNumChildren(kNodeType_Any);
	for (i=0;i<n;i++)
	{
		IDocNode *pChild = pNode->GetChildAt(i, kNodeType_Any); 
		RemoveNodeTraceObjects(pChild, nodeObjects);
	}
	// remove node from lookup
	Lookup::DeregisterNodeExtInst(pNode);
	IBaseInstance *pBaseObject = pNode->GetNodeObject();
	if (pBaseObject!=NULL)
	{
		treemap.erase(pBaseObject);
		nodeObjects.push_back(pBaseObject);
	}

	return bRes;
}

//
// removes a node and all of its children
//
bool Document::RemoveNode(IDocNode *pNode)
{
	bool bRes = false;
	std::vector<IBaseInstance *> instances;
	bRes = RemoveNodeTraceObjects(pNode, instances);
	// this would delete the Document root node, not very good
	IDocNode *pParent = pNode->GetParent();
	if (pParent!=NULL)
	{
		pNode->GetParent()->RemoveChild(pNode);
	} else
	{
	}
	pNode->Dispose();
		
	// TODO: Clear up and dispose the base instances
	// NOTE: We should not call PostInitialize from here, the document only works with the data!
	if (pParent != NULL)
	{
		// reinitialize data, if node had something cached the object must recap from changes
		pDocumentController->PostInitializeNode(pParent);
		
	}
	return bRes;
}

//
// removes nodes based on object input
//
bool Document::RemoveObject(IBaseInstance *pObject)
{
	bool bRes = false;
	IDocNode *pNode = FindNode(pObject);
	bRes = RemoveNode(pNode);
	return bRes;
}

void Document::DumpNode(IDocNode *pNode)
{
	ILogger *pLogger = Logger::GetLogger("Dump");
	IBaseInstance *pInstance = pNode->GetNodeObject();

	if (pInstance != NULL)
	{
		// Dump object
//		pLogger->Debug("Inst: %s (cls: %s)",pInstance->GetAttributeValue("name"),pInstance->GetDefinition()->GetAttributeValue(kObjectAttribute_Identifier));
		pLogger->Debug("Inst: %p,%d,%s",pInstance,pInstance->GetInstanceType(),pInstance->GetFullyQualifiedName());
		pLogger->Enter();
		//pInstance->Dump();
		pLogger->Leave();
	}
	int n,i;
	n = pNode->GetNumChildren();
	for (i=0;i<n;i++)
	{
		IDocNode *pChild;
		pChild = pNode->GetChildAt(i);
		pLogger->Enter();
		DumpNode(pChild);
		pLogger->Leave();
	}
}

void Document::DumpRenderTree()
{
	pLogger->Debug("Dumping Document");
	DumpNode(root);
	pLogger->Debug("Dump done");
}


// traversing with IPluginObject as input
int Document::GetNumChildren(IPluginObjectInstance *pObject)
{
	int iRes = -1;
	IDocNode *pNode = Lookup::GetNodeForExtInst(pObject->GetExtObject());
	if (pNode != NULL)
	{
		iRes = pNode->GetNumChildren();
	} else
	{
		SetYaptLastError(kErrorClass_General, kError_ObjectNotFound);
	}
	return iRes;
}
int Document::GetNumChildren(IPluginObjectInstance *pObject, kNodeType ofType)
{
	int iRes = -1;
	IDocNode *pNode = Lookup::GetNodeForExtInst(pObject->GetExtObject());
	if (pNode != NULL)
	{
		iRes = pNode->GetNumChildren(ofType);
	} else
	{
		SetYaptLastError(kErrorClass_General, kError_ObjectNotFound);
	}
	return iRes;
}

IPluginObjectInstance* Document::GetChildAt(IPluginObjectInstance *pObject, int index, kNodeType ofType)
{
	IPluginObjectInstance *result = NULL;
	IDocNode *pNode = Lookup::GetNodeForExtInst(pObject->GetExtObject());
	if (pNode != NULL)
	{
		pNode = pNode->GetChildAt(index,ofType);
		if (pNode != NULL)
		{
			result = dynamic_cast<IPluginObjectInstance *>(pNode->GetNodeObject());
		} else
		{
			SetYaptLastError(kErrorClass_General, kError_NoChildOfType);
		}
	} else
	{
		SetYaptLastError(kErrorClass_General, kError_ObjectNotFound);
	}
	return result;
}

IPluginObjectInstance* Document::GetChildAt(IPluginObjectInstance *pObject, int index)
{
	IPluginObjectInstance *result = NULL;
	IDocNode *pNode = Lookup::GetNodeForExtInst(pObject->GetExtObject());
	if (pNode != NULL)
	{
		pNode = pNode->GetChildAt(index);
		if (pNode != NULL)
		{
			result = dynamic_cast<IPluginObjectInstance*>(pNode->GetNodeObject());
		} else
		{
			SetYaptLastError(kErrorClass_General, kError_NoChildOfType);
		}
	} else
	{
		SetYaptLastError(kErrorClass_General, kError_ObjectNotFound);
	}
	return result;
}

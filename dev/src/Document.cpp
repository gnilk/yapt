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
- 26.04.12, FKling, Rewrote include handling, now uses lazy evaluation and ability to declare unbound properties on load
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
Document::Document() :
	BaseInstance(kInstanceType_Document)
{
  Initialize();
}


Document::Document(IContext *pContext) :
	BaseInstance(kInstanceType_Document)
{
	Initialize();
	SetContext(pContext);	// set the context in the base class 
	// setup default objects
	// currently we setup a resource container and a render node

	timeline = NULL;	// time line is optional

	// Since we "inject" predefined stuff in the object tree it is important that we force
	// resources first. During initalization the tree is simply traversed - resource should be initialized
	// first.

	// Force creation of resource container
	resources = new ResourceContainer();//new DocNode(dynamic_cast<IDocument *> (this));
	resources->SetContext(pContext);
	resources->AddAttribute("name","resources");
	AddObjectToTree(dynamic_cast<IBaseInstance *>(this), resources, kNodeType_ResourceContainer);

	BaseInstance *pDummy = new BaseInstance(kInstanceType_RenderNode);
	pDummy->SetContext(pContext);
	pDummy->AddAttribute("name","render");
	renderRoot = AddObjectToTree(dynamic_cast<IBaseInstance *>(this), pDummy, kNodeType_RenderNode);

}

Document::~Document()
{
	DisposeTree(root);
}
void Document::DisposeTree(IDocNode *root)
{
	int n = root->GetNumChildren();
	for(int i=0;i<n;i++) {
		DisposeTree(root->GetChildAt(i));
	}
	delete root;
}
void Document::Initialize() 
{
	pLogger = Logger::GetLogger("Document");
	pContext = NULL;

	this->pDocumentController = NULL;
	std::string fqName; 

	// Set up root node, the document is always the root node of the tree
	root = dynamic_cast<IDocNode *>(new DocNode(dynamic_cast<IDocument *>(this)));
	root->SetNodeObject(dynamic_cast<IBaseInstance *>(this), kNodeType_Document);
	AddAttribute("name","doc");
	fqName = BuildQualifiedName(root);
	SetFullyQualifiedName(fqName.c_str());
	treemap.insert(BaseNodePair(dynamic_cast<IBaseInstance *>(this),root));
}

void Document::SetDocumentController(IDocumentController *pDocumentController)
{
	this->pDocumentController = pDocumentController;
}

IDocumentController *Document::GetDocumentController()
{
  return pDocumentController;
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

ITimeline *Document::GetTimeline() {
  if (timeline == NULL) {
    // Lazy create the time line object
	  timeline = new Timeline();
	  timeline->SetContext(pContext);
	  timeline->AddAttribute("name","timeline");
	  AddObjectToTree(dynamic_cast<IBaseInstance *>(this), timeline, kNodeType_Timeline);
  }
	return timeline;
}

bool Document::HasTimeline() {
  return ((timeline==NULL)?false:true);
}

IResourceContainer *Document::GetResources()
{
  if (resources == NULL) {
	  resources = new ResourceContainer();//new DocNode(dynamic_cast<IDocument *> (this));
	  resources->SetContext(pContext);
	  resources->AddAttribute("name","resources");
	  AddObjectToTree(dynamic_cast<IBaseInstance *>(this), resources, kNodeType_ResourceContainer);
  }
	return dynamic_cast<IResourceContainer *>(resources);
}

IBaseInstance *Document::SearchFromNode(IDocNode *pRootNode, const char *name) {
	int i;
	for (i=0;i<pRootNode->GetNumChildren();i++)
	{
		IDocNode *pChild = pRootNode->GetChildAt(i);
		IBaseInstance *pObject = pChild->GetNodeObject();
    if (pObject == NULL) continue;
    // Do depth first search -
    IBaseInstance *pRet = SearchFromNode(pChild, name);
    if (pRet != NULL) return pRet;

    // Only pluggable objects may be referenced
    PluginObjectInstance *pNamedInstance = dynamic_cast<PluginObjectInstance *>(pObject);
    if(pNamedInstance == NULL) continue;

    const char *sName = pNamedInstance->GetInstanceName();
    if (!StrConfCaseCmp(name,sName)) {
    	return pChild->GetNodeObject();
    }
	}
	return NULL;
}
IBaseInstance *Document::GetObjectFromSimpleName(const char *name) {
	return SearchFromNode(GetTree(),name);
}
IBaseInstance *Document::GetObject(const char *fullQualifiedName)
{
	// TODO: Need lookup str::BasInstance	
	//return Lookup::GetBaseFromString(fullQualifiedName);
	return NULL;
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
	
	//pLogger->Debug("AddNode, node=(%p) for object=%p",pNode, pObject);

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

	// pLogger->Debug("AddObjectToTree, nodeType=%d",nodeType);
	// if (nodeType == kNodeType_PropertyInstance) {
	// 	char tmp[256];
	// 	IPropertyInstance *pInst = dynamic_cast<IPropertyInstance *>(object);
	// 	pLogger->Debug("AddObjectToTree, Property Type=%d",pInst->GetPropertyType());
	// 	pInst->GetPropertyTypeName(tmp,256);
	// 	pLogger->Debug("AddObjectToTree, Property TypeName=%s",tmp);
	// 	pInst->GetValue(tmp,256);
	// 	pLogger->Debug("AddObjectToTree, Property Value=%s",tmp);
	// }


	// Ensure we have an attribute "name"
	if (!object->GetAttribute("name"))
	{
		const char *noname = yapt::GetYaptSystemInstance()->GetConfigValue("Document.DefaultNodeName","noname");
		object->AddAttribute("name",noname);
	}

	parentNode = FindNode(parent);
	if (parentNode != NULL)
	{
		//pLogger->Debug("Parent node ok, addNode");
		pNode = AddNode(parentNode, object, nodeType);

		BaseInstance *pBase = dynamic_cast<BaseInstance *>(object);
		std::string qName = BuildQualifiedName(pNode);

	    // Don't assign funky names to implicit nodes (doc, resources and render)
	    if ((nodeType != kNodeType_Document) &&
	        (nodeType != kNodeType_ResourceContainer) &&
	        (nodeType != kNodeType_Timeline) &&
	    	(nodeType != kNodeType_RenderNode)) {


	    	// NOTE [2014-08-19]: This is not required (I think) but I don't understand why I did it in the first place


			  // object with this name already registered?
			  // append a number to the name until it gets unique
			  // if (Lookup::GetBaseFromString(qName) != NULL)
			  // {
				 //  int extCount = 1;
				 //  // Can't reuse the const char * return value since if we update the
				 //  // attribute in the while loop it will be deallocated
				 //  std::string baseName(pBase->GetAttributeValue("name"));

				 //  // will update the name with a numbered version
				 //  while (Lookup::GetBaseFromString(qName) != NULL)
				 //  {				
					//   char tmp[32];

					//   std::string curName(baseName);
					//   snprintf(tmp, 32, "%d", extCount);
					//   curName.append(tmp);		// itoa not supported..
					//   pBase->AddAttribute("name",curName.c_str());
					//   qName = BuildQualifiedName(pNode);

					//   extCount++;
				 //  }
			  // }
	    } // node type

//		pLogger->Debug("Setting Qualified name");
		pBase->SetFullyQualifiedName(qName.c_str());

		pLogger->Debug("AddObjectToTree, %s",qName.c_str());

		// add a lookup for this one..
//		Lookup::RegisterStrBase(qName, object);
	}

	return pNode;
}

IDocNode *Document::AddObject(IBaseInstance *parent, IBaseInstance *object, kNodeType nodeType)
{
	IDocNode *pNode = AddObjectToTree(parent,object, nodeType);
	return pNode;
}

IDocNode *Document::AddRenderObject(IBaseInstance *parent, IBaseInstance *object)
{
	IDocNode *pNode = AddObjectToTree(parent,object,kNodeType_ObjectInstance);
	return pNode;
}

IDocNode *Document::AddToTimeline(IBaseInstance *object) {
	int n = GetTimeline()->GetNumExecutors();
	pLogger->Debug("Add to timeline, for doc %p - num exec before: %d", this, n);
	for(int i=0;i<n;i++) {		
	    ITimelineExecute *pExec = dynamic_cast<ITimelineExecute *>(timeline->GetExecutorAtIndex(i));
		pLogger->Debug("  %d, %s", i, pExec->GetObjectName());
	}
	// GetTimeline()->GetNodeObject()
	//IDocNode *timeLineNode = FindNode(timeline);
	DumpNode(FindNode(timeline));


	return AddObjectToTree(timeline, object, kNodeType_ObjectInstance);
}

void Document::AddResourceObject(IBaseInstance *parent, IBaseInstance *object)
{
	IDocNode *pNode = AddObjectToTree(parent,object,kNodeType_ObjectInstance);
	IPluginObjectInstance *pInst = dynamic_cast<IPluginObjectInstance *>(object);
	resources->AddResourceInstance(pInst);
}

IDocNode *Document::AddMetaObject(IBaseInstance *parent) 
{
	IBaseInstance *pMeta = new MetaInstance(this);
	IDocNode *pNode = AddObjectToTree(parent, pMeta, kNodeType_Meta);
	return pNode;
}

IDocNode *Document::AddCommentObject(IBaseInstance *parent) {
	IBaseInstance *pComment = new CommentInstance();
	IDocNode *pNode = AddObjectToTree(parent, pComment, kNodeType_Comment);
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
	    if (pInstance->GetInstanceType() == kInstanceType_MetaNode) {
	      pLogger->Debug("**: META :**");
	      IMetaInstance *pMeta = dynamic_cast<IMetaInstance *>(pInstance);
	      pMeta->GetDocument()->DumpRenderTree();
	    }
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


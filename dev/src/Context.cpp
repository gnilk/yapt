/*-------------------------------------------------------------------------
 File    : $Archive: Context.cpp $
 Author  : $Author: Fkling $
 Version : $Revision: 1 $
 Orginal : 2009-10.08, 15:50
 Descr   : Placeholder for related objects
 
	! Document
	! DocumentController
	- Lookup
 
 Refactoring of control out of the document
 
 Modified: $Date: $ by $Author: Fkling $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
 </pre>
 
 
 \History
 - 08.10.09, FKling, Implementation
 - 20.10.09, FKling, Renamed to Context
 - 03.11.09, FKling, Added ability to associate external objects with the context
 - 13.11.09, FKling, Added the ability to set prefix on strings (used during import includes)
 
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


Context::Context()
{
	pLogger = Logger::GetLogger("Context");


	pDocument = new Document(dynamic_cast<IContext *>(this));
	// link document and controller together via the interfaces
	// this is ok, interfaces are not circular depending only the current implementation
	pDocumentController = new DocumentController(dynamic_cast<IDocument *>(pDocument));	
	pDocument->SetDocumentController(pDocumentController);
	pDocument->SetContext(dynamic_cast<IContext *>(this));
	namePrefix = "";
}
Context::~Context()
{
	delete pDocument;
	delete pDocumentController;
	// TODO: clean up context param stack

}

void Context::SetDocument(IDocument *pDocument)
{
	this->pDocument = dynamic_cast<Document *>(pDocument);
}
void Context::SetDocumentController(DocumentController *pController)
{
	this->pDocumentController = pController;
}

IDocument *Context::GetDocument()
{
	return dynamic_cast<IDocument *>(pDocument);
}
IDocumentController *Context::GetDocumentController()
{
	return dynamic_cast<IDocumentController *>(pDocumentController);
}
void Context::SetObject(const char *name, void *pObject)
{
	if (objects.find(name) == objects.end())
	{
		objects.insert(CtxNameObjectPair(name,pObject));
	} else
	{
		objects[name]=pObject;
	}

}
void *Context::GetObject(const char *name)
{
	void *pObject= NULL;
	if (objects.find(name) != objects.end())
	{
		pObject=objects[name];
	}
	return pObject;
}

void Context::PushRenderObject(IBaseInstance *obj) {
	renderObjectStack.push(obj);
}

void Context::PopRenderObject() {
	renderObjectStack.pop();
}

IBaseInstance *Context::TopRenderObject() {
	return renderObjectStack.top();
}

void Context::PushContextParamObject(void *pObject, const char *name)
{
	CtxNameObjectPair ctxObject(std::string(name), pObject);
	this->contexParamObjectStack.push(ctxObject);
//	this->contextParamObject = pObject;
}

void Context::PopContextParamObject() {
	this->contexParamObjectStack.pop();
}

void *Context::TopContextParamObject() {
	//return this->contextParamObject;
	return (void *)this->contexParamObjectStack.top().second;	
}


void Context::SetNamePrefix(const char *prefix)
{
	namePrefix = std::string(prefix);
}

char *Context::GetNamePrefix(char *pdest, int nmaxlen)
{
	strncpy(pdest, namePrefix.c_str(), nmaxlen);
	return pdest;
}

// creates a string on the form <prefix>.<name>
char *Context::CreatePrefixName(const char *name, char *prefixedname, int nmaxlen)
{
	snprintf(prefixedname, nmaxlen, "%s.%s",namePrefix.c_str(), name);
	return prefixedname;
}

//
// Node/Instance cache routines
//
void Context::AddNode(IBaseInstance *instance, IDocNode *node) {

	pLogger->Debug("AddNode, %s (node=%p, context=%p)",instance->GetFullyQualifiedName(), node, this);

	baseNodeMap.insert(BaseNodePair(instance,node));
}

IDocNode *Context::FindNode(IBaseInstance *pObject) {
	if (baseNodeMap.find(pObject)!=baseNodeMap.end())
	{
		return baseNodeMap[pObject];
	} else
	{
		pLogger->Debug("Failed to find node for object=%p",pObject);
	}
	return NULL;	
}

void Context::EraseNode(IBaseInstance *instance) {
	baseNodeMap.erase(instance);
}

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
	pDocument = new Document(dynamic_cast<IContext *>(this));
	// link document and controller together via the interfaces
	// this is ok, interfaces are not circular depending only the current implementation
	pDocumentController = new DocumentController(dynamic_cast<IDocument *>(pDocument));	
	pDocument->SetDocumentController(pDocumentController);
	pDocument->SetContext(dynamic_cast<IContext *>(this));
	namePrefix = "";
	contextParamObject = NULL;
}
Context::~Context()
{
	delete pDocument;
	delete pDocumentController;
	// param object is not allocated by us..
}

void Context::SetDocument(Document *pDocument)
{
	this->pDocument = pDocument;
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

void Context::SetContextParamObject(void *pObject)
{
	this->contextParamObject = pObject;
}

void *Context::GetContextParamObject() {
	return this->contextParamObject;
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

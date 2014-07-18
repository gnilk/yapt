/*-------------------------------------------------------------------------
File    : $Archive: yBaseInstance.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : Baseclass for all document instance parts, i.e. all nodes in the document
          has the base instance as the base class. Event the document it self.
	  The baseinstance holds the attributes, thus all document objects can
	  hold attributes.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.09.09, FKling, Implementation
- 21.10.09, FKling, Context assigned upon creation of object
---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

#include "yapt/logger.h"

#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#include <malloc.h>
#define snprintf _snprintf
#endif

using namespace yapt;


BaseInstance::BaseInstance(kInstanceType mType)
{
	type = mType;
	pContext=NULL;
	qualifiedName = strdup("__no_qualified_name_this_is_a_bug__");
	pContext = yapt::GetYaptSystemInstance()->GetCurrentContext();
}
BaseInstance::~BaseInstance()
{
	free (qualifiedName);
	for(int i=0;i<attributes.size();i++) {
		delete attributes[i];		
	}

}

kInstanceType BaseInstance::GetInstanceType()
{
	return type;
}
const char *BaseInstance::GetFullyQualifiedName()
{
	return qualifiedName;
}
void BaseInstance::SetFullyQualifiedName(const char *qName)
{
	free (qualifiedName);
	qualifiedName = strdup (qName);
}


int BaseInstance::GetNumAttributes()
{
	return (int)attributes.size();
}

Attribute *BaseInstance::GetAttributeAsClass(int index)
{
	Attribute *result;
	if (index < (int)attributes.size())
	{
		result = attributes[index];
	}
	return (result);
}

Attribute *BaseInstance::GetAttributeAsClass(const char *name)
{
	Attribute *result = NULL;
	size_t i;
	for (i=0;(i<attributes.size()) && (result == NULL);i++)
	{
		//if (!strcmp(name, attributes[i]->GetName()))
		if (!StrConfCaseCmp(name, attributes[i]->GetName()))
		{
			result = attributes[i];
		}
	}
	return result;
}

IAttribute *BaseInstance::GetAttribute(int index)
{
	return dynamic_cast<IAttribute *>(GetAttributeAsClass(index));
}

IAttribute *BaseInstance::GetAttribute(const char *name)
{
	return dynamic_cast<IAttribute*>(GetAttributeAsClass(name));
}

const char *BaseInstance::GetAttributeValue(const char *name)
{
	IAttribute *a;
	a = GetAttribute(name);
	if (!a) return NULL;
	return a->GetValue();
}

const char *BaseInstance::GetAttributeValue(const char *name, const char *sDefault)
{
	IAttribute *a;
	a = GetAttribute(name);
	if (!a) return sDefault;
	return a->GetValue();	
}

IAttribute *BaseInstance::AddAttribute(const char *name, const char *value)
{
	Attribute *attr = GetAttributeAsClass(name);
	if (attr == NULL)
	{
		// does not exists
		attr = new Attribute(name,value);
		attributes.push_back(attr);
	} else
	{
		attr->SetValue(value);
	}
	OnAttributeChanged(attr);
	return dynamic_cast<IAttribute *>(attr);
}

IAttribute *BaseInstance::UpdateAttribute(const char *name, const char *value)
{
	return AddAttribute(name, value);
}

void BaseInstance::OnAttributeChanged(Attribute *pAttribute)
{
	// do nothing, inherited classes can override this one

}

IContext *BaseInstance::GetContext()
{
	return pContext;
}

void BaseInstance::SetContext(IContext *pContext)
{
	this->pContext = pContext;
}

//
// TODO: Move this one out of here
// Compare two string with respect to case configuration
//
int BaseInstance::StrConfCaseCmp(const char *sA, const char *sB)
{
	int iRes = 1;
	bool bCaseSensitive = yapt::GetYaptSystemInstance()->GetConfigBool(kConfig_CaseSensitive,false);
	if (!bCaseSensitive)
	{
		size_t i;
		char *ta = (char *)alloca(strlen(sA)+1);
		char *tb = (char *)alloca(strlen(sB)+1);
		for (i=0;i<strlen(sA);i++) ta[i]=toupper(sA[i]);
		ta[i]='\0';
		for (i=0;i<strlen(sB);i++) tb[i]=toupper(sB[i]);
		tb[i]='\0';
		iRes = strcmp(ta,tb);
	} else
	{
		iRes = strcmp(sA,sB);
	}
	return iRes;
}

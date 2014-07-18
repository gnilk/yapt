/*-------------------------------------------------------------------------
 File    : $Archive: DocNode.cpp $
 Author  : $Author: Fkling $
 Version : $Revision: 1 $
 Orginal : 2009-09-17, 15:50
 Descr   : Implements the Document Node class functionality. A Doc Node is an internal representation of
 a node (either XML or other) in the rendering tree. A Node holds a pointer to a user object of type IBaseInstance
 
 Modified: $Date: $ by $Author: Fkling $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
 </pre>
 
 
 \History
 - 08.10.09, FKling, Refactored here from Document
 
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


//////////////////////////////////////////////////////////////////
//
// -- Document node
//
DocNode::DocNode(IDocument *doc)
{
	document = doc;
	userObject = NULL;
	parent = NULL;
	nodeType = kNodeType_Regular;	// all nodes are by default regular nodes
}

DocNode::~DocNode()
{
	if (userObject != NULL)
	{
		delete userObject;
	}	
}

bool DocNode::Dispose()
{
	if (children.size() > 0)
	{
		size_t i;
		for (i=0;i<children.size();i++)
		{
			children[i]->Dispose();
		}
		children.clear();
	}
	delete this;
	return true;
}

IDocNode *DocNode::GetParent()
{
	return parent;
}

void DocNode::AddChild(IDocNode *child)
{
	children.push_back(child);
	dynamic_cast<DocNode *>(child)->parent = this;
}

void DocNode::RemoveChild(IDocNode *child)
{
	std::vector<IDocNode *>::iterator it;
	
	for (it=children.begin();it!=children.end();it++)
	{
		if (*it == child)
		{
			children.erase(it);
			break;
		}
	}
}
// count sub node of a specific type
// TODO: Store num children per type in a specific array
int DocNode::GetNumChildren(kNodeType ofType)
{
	size_t i;
	int num;
	num=0;
	for (i=0;i<children.size();i++)
	{
		if ((children[i]->GetNodeType() == ofType) || (ofType == kNodeType_Any)) 
		{
			num++;
		}
	}
	return num;
}
IDocNode *DocNode::GetChildAt(int idx, kNodeType ofType)
{
	size_t i;
	int num;
	num=0;
	for (i=0;i<children.size();i++)
	{
		if ((children[i]->GetNodeType() == ofType) || (ofType == kNodeType_Any)) 
		{
			if (num==idx)
			{
				return dynamic_cast<IDocNode *>(children[i]);
			}
			num++;
		}
	}
	return NULL;
	
}

int DocNode::GetNumChildren()
{
	return (int)children.size();
}

IDocNode *DocNode::GetChildAt(int idx)
{
	return children.at(idx);
}

IDocument *DocNode::GetRootDocument()
{
	return document;
}

IBaseInstance *DocNode::GetNodeObject()
{
	return userObject;
}

kNodeType DocNode::GetNodeType()
{
	return nodeType;
}
void DocNode::SetNodeObject(IBaseInstance *pObject, kNodeType type)
{
	nodeType = type;
	userObject = pObject;
}


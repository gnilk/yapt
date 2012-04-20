/*-------------------------------------------------------------------------
File    : $Archive: ExportXML.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-13, 15:50
Descr   : Simple and very small XML Exporter

Will change input structure and flatten the document (if using includes)
This is because we can't track includes

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 13.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <iostream>
#include <stdio.h>
#include <stack>
#include <string>
#include <map>
#ifdef WIN32
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#define snprintf _snprintf
#include <windows.h>
#else
#endif



#include "yapt/logger.h"
#include "yapt/ySystem.h"	
#include "yapt/ySystem_internal.h"
#include "yapt/ExportXML.h"
using namespace yapt;

#include "noice/io/io.h"
#include "noice/io/ioutils.h"
//#include "yapt/URLParser.h"
using namespace noice::io;




//static char *lRootTags[]={"yapt2",NULL};
//static kParserState lRootChange[]={kParserState_Doc};
//static char *lDocTags[]={"resources","render","include",NULL};
//static kParserState lDocChange[]={kParserState_Resources, kParserState_Render, kParserState_Include};
//static char *lResourceTags[]={"object",NULL};
//static kParserState lResourceChange[]={kParserState_Object};
//static char *lRenderTags[]={"object",NULL};
//static kParserState lRenderChange[]={kParserState_Object};
//static char *lObjectTags[]={"object","property",NULL};
//static kParserState lObjectChange[]={kParserState_Object, kParserState_Property};


ExportXML::ExportXML(IDocument *pDocument)
{
	this->pDoc = pDocument;
	this->pStream = NULL;
	pLogger = Logger::GetLogger("ExportXML");
	iIndent = 0;
	sIndent = std::string("");
}	
ExportXML::~ExportXML()
{
}
void ExportXML::PushIndent()
{
	iIndent += 2;
	CreateIndentString();
}
void ExportXML::PopIndent()
{
	iIndent-=2;
	CreateIndentString();
}
void ExportXML::CreateIndentString()
{
	char tmp[256];
	for(int i=0;i<iIndent;i++)
	{
		tmp[i]=' ';
		tmp[i+1]='\0';
	}
	sIndent = std::string(tmp);
}
void ExportXML::WritePropertyData(IBaseInstance *pBase)
{
	IPropertyInstance *pInst = dynamic_cast<IPropertyInstance *>(pBase);
	if (pInst != NULL)
	{
		char tmp[256];
		pInst->GetValue(tmp, 256);
		pStream->Write(tmp,(int)strlen(tmp));
	} else
	{
		pLogger->Error("Not a property, identification object type mismatch");
	}
}

void ExportXML::Begin(const char *tagName, IBaseInstance *pBase, bool bNewLine)
{
	char tmp[256];
	tagStack.push(std::string(tagName));
	tagNewLineStack.push(bNewLine);
	snprintf(tmp,256,"%s<%s",sIndent.c_str(), tagName);
	pStream->Write(tmp,(int)strlen(tmp));
	if (pBase->GetNumAttributes() > 0)
	{
		int i,n;
		n = pBase->GetNumAttributes();
		for(i=0;i<n;i++)
		{
			IAttribute *pAttr = pBase->GetAttribute(i);
			snprintf(tmp,256," %s=\"%s\"",pAttr->GetName(),pAttr->GetValue());
			pStream->Write(tmp,(int)strlen(tmp));
		}
	}
	snprintf(tmp,256,">");
	pStream->Write(tmp,(int)strlen(tmp));
	if (bNewLine)
	{
		snprintf(tmp,256,"\n");
		pStream->Write(tmp,(int)strlen(tmp));
	}
	PushIndent();
}

void ExportXML::End()
{
	bool bNewLine = true;
	char tmp[256];
	std::string tagName = tagStack.top();
	bNewLine = tagNewLineStack.top();
	PopIndent();
	tagNewLineStack.pop();
	tagStack.pop();

	// only write indent if start tag ended with a new line
	if (bNewLine)
	{
		snprintf(tmp,256,"%s</%s>\n",sIndent.c_str(),tagName.c_str());
	} else
	{
		snprintf(tmp,256,"</%s>\n",tagName.c_str());
	}
	pStream->Write(tmp,(int)strlen(tmp));
}

bool ExportXML::WriteNode(IDocNode *pNode)
{
	bool bSkipEndTag = false;
	IBaseInstance *pBase = pNode->GetNodeObject();
	if (pBase != NULL)
	{
		switch(pBase->GetInstanceType())
		{
		case kInstanceType_Document :
			Begin(kDocument_RootTagName, pBase,true);
			break;
		case kInstanceType_ResourceContainer:
			if (pNode->GetNumChildren())
			{
				Begin(kDocument_ResourceTagName, pBase,true);
			} else
			{
				bSkipEndTag = true;
			}
			break;
		case kInstanceType_RenderNode:
			Begin(kDocument_RenderTagName, pBase,true);
			break;
		case kInstanceType_Object:
			Begin(kDocument_ObjectTagName, pBase,true);
			break;
		case kInstanceType_Property:
			Begin(kDocument_PropertyTagName,pBase,false);
			WritePropertyData(pBase);
			break;
		default:
			pLogger->Error("Unknown instance type: %d, can't export node");
			bSkipEndTag = true;
			break;
		}

		int i,n;
		n=pNode->GetNumChildren();
		for(i=0;i<n;i++)
		{
			WriteNode(pNode->GetChildAt(i));
		}
		if (!bSkipEndTag)
		{
			End();
		}
	}
	return true;
}
bool ExportXML::ExportDocument()
{
	IDocNode *pNode = pDoc->GetTree();
	return WriteNode(pNode);
}

bool ExportXML::ExportToStream(noice::io::IStream *pStream)
{
	bool bRes = false;
	this->pStream = pStream;
	pStream->Open(kStreamOp_WriteOnly);
	bRes = ExportDocument();
	pStream->Close();
	return bRes;
}	
bool ExportXML::ExportToFile(const char *sFilename)
{
//	BasicFileStream *stream = new BasicFileStream(sFilename);
	noice::io::IStreamDevice *pDevice = yapt::GetYaptSystemInstance()->GetIODevice("file");
	noice::io::IStream *pStream = pDevice->CreateStream(sFilename,0);
	return ExportToStream(pStream);
}

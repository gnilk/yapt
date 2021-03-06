/*-------------------------------------------------------------------------
File    : $Archive: BasicFileStream.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : 	Non validating XML importer using Expat
    Note: "Schema" explicitly defined within this file.

    Supports includes and inplace merging.

    NOTE: If you will include a file and serialize the document again, 
    the include will have been merged with the rest of the document.

    TODO: By adding transparent DOC:Nodes we can easily preserve both comments and
    include statements. -> CONSIDER!

    Includes might be useful for handwritten scripts


Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
- Solve dependency on BasicFileStream
- Put in a DLL
! Move Node initialization logic out of here (node initialization is now a document post process step)
</pre>


\History
- 17.10.09, FKling, Implementation
- 26.04.12, FKling, Rewrote include handling, now uses lazy evaluation and ability to declare unbound properties on load
                    Enables the document to keep track of include when saving!

---------------------------------------------------------------------------*/
#include <iostream>
#include <stdio.h>
#include <stack>
#include <string>
#include <map>
#ifdef WIN32
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "expat/lib/expat.h"
#else
#include <expat.h>
#endif

#include "yapt/logger.h"
#include "yapt/ySystem.h"	

#include "yapt/ySystem_internal.h"

//#include "yapt/URLParser.h"
#include "yapt/ExpatXMLImporter.h"

using namespace yapt;


#include "noice/io/io.h"
#include "noice/io/ioclasses.h"
#include "noice/io/ioutils.h"

using namespace noice::io;


// Defines list of tags allowed and at which level, second list (synchronized to tag names) define the parser state for a sub-tag
static const char *lRootTags[]={kDocument_RootTagName,NULL};
static kParserState lRootChange[]={kParserState_Doc};
static const char *lDocTags[]={kDocument_ResourceTagName,kDocument_RenderTagName,kDocument_IncludeTagName,kDocument_TimelineTagName, kDocument_SignalsTagName, NULL};
static kParserState lDocChange[]={kParserState_Resources, kParserState_Render, kParserState_Include, kParserState_Timeline, kParserState_Signals};
static const char *lResourceTags[]={kDocument_ObjectTagName,NULL};
static kParserState lResourceChange[]={kParserState_Object};
static const char *lRenderTags[]={kDocument_ObjectTagName,NULL};
static kParserState lRenderChange[]={kParserState_Object};
static const char *lTimelineTags[]={kDocument_ExecuteTagName,NULL};
static kParserState lTimelineChange[]={kParserState_Timeline_Execute};

static const char *lSignalsTags[]={kDocument_SignalChannelTagName,NULL};
static kParserState lSignalsChange[]={kParserState_SignalChannel};

static const char *lSignalChannelTags[]={kDocument_SignalTagName,NULL};
static kParserState lSignalChannelChange[]={kParserState_Signal};

static const char *lObjectTags[]={kDocument_ObjectTagName,kDocument_PropertyTagName,NULL};
static kParserState lObjectChange[]={kParserState_Object, kParserState_Property};

// TODO: REMOVE
//static IDocument *glbDocument = NULL;

ExpatXMLParser::ExpatXMLParser(ISystem *pSys)
{
  this->ySys = pSys;
  depthPtr = 0;
  pLogger = Logger::GetLogger("expat");
  bCaptureCData = false;
  pCdataProperty = NULL;
  pDocument = NULL;
  pRoot = NULL;
}
ExpatXMLParser::~ExpatXMLParser()
{
}

void ExpatXMLParser::ParseError(const char *errString)
{
  fprintf(stderr,"%s at line %d\n",XML_ErrorString(XML_GetErrorCode(parser)),(int)XML_GetCurrentLineNumber(parser));
  pLogger->Error("%s  (%s at line %d)",
    errString,
    XML_ErrorString(XML_GetErrorCode(parser)),
    XML_GetCurrentLineNumber(parser));
}

// helper
static int IsInList(const char *name, const char **list)
{
  int i=0;
  while(list[i]!=NULL)
  {
    if (!strcmp(name,list[i])) return i;
    i++;
  }
  return -1;
}
static int GetAttributeIndex(const char *name, const char **atts)
{
  int res = -1;
  int i =0;
  while(atts[i]!=NULL)
  {
    if (!strcmp(name,atts[i])) return i;
    i+=2;
  }

  return res;
}

#define CHECK_STATE_CHANGE(__name,__tags,__next) \
{ \
  int idx = IsInList(__name, __tags); \
  if (idx != -1) \
{ \
  stateStack.push(__next[idx]); \
  bRes = true; \
}\
}

//
// Verifies if an element (name) is allowed as a sub in the current state
//
bool ExpatXMLParser::IsElementAllowed(const char *name)
{
  bool bRes = false;
  switch(stateStack.top())
  {
  case kParserState_Root:
    CHECK_STATE_CHANGE(name,lRootTags, lRootChange);
    break;
  case kParserState_Doc :
    CHECK_STATE_CHANGE(name, lDocTags, lDocChange);
    break;
  case kParserState_Configuration :
    // nothing allowed to sub to configuration
    pLogger->Error("Illegal parser state, configuration not allowed");
    break;
  case kParserState_Resources :
    CHECK_STATE_CHANGE(name,lResourceTags, lResourceChange);
    break;
  case kParserState_Timeline:
    CHECK_STATE_CHANGE(name, lTimelineTags, lTimelineChange);
    break;
  case kParserState_Signals:
    CHECK_STATE_CHANGE(name, lSignalsTags, lSignalsChange);
    break;
  case kParserState_SignalChannel:
    CHECK_STATE_CHANGE(name, lSignalChannelTags, lSignalChannelChange);
    break;
  case kParserState_Render :
    CHECK_STATE_CHANGE(name,lRenderTags, lRenderChange);
    break;
  case kParserState_Object :
    CHECK_STATE_CHANGE(name,lObjectTags, lObjectChange);
    break;
  case kParserState_Timeline_Execute :
  case kParserState_Property :
    // nothing allowed as sub
    break;
  default:
    pLogger->Error("Unknown parser state: %d",(int)stateStack.top());
  }
  return bRes;
}

//
// Includes data from an external file
//
void ExpatXMLParser::IncludeFromURL(const char *url, const char **atts)
{
  yapt::ISystem *system = GetYaptSystemInstance();
  noice::io::IStream *pStream = system->CreateStream(url, 0);
  if (pStream != NULL)
  {
    pLogger->Debug("Processing include directive for '%s'",url);

    if (!pStream->Open(kStreamOp_ReadOnly)) {
      pLogger->Error("Failed to open stream for '%s'", url);
      exit(1);
    }
    ExpatXMLParser *xml = new ExpatXMLParser(system);
    IMetaInstance *pMeta = dynamic_cast<IMetaInstance *>(instanceStack.top());
    pMeta->GetDocument();
    xml->SetRoot(pMeta);

    IDocument *oldDocument = ySys->GetActiveDocument();
    ySys->SetActiveDocument(pMeta->GetDocument());

    pLogger->Debug("Swapping docs, old=%p, new=%p", oldDocument, pMeta->GetDocument());

    // Idea on how to support name prefixing during node creation
    char prefixCurrent[128];
    int newPrefixIdx = GetAttributeIndex("prefix",atts);
    if(newPrefixIdx > 0)
    {
      yapt::IContext *pContext = system->GetCurrentContext();
      pContext->GetNamePrefix(prefixCurrent, 128);
      pContext->SetNamePrefix(atts[newPrefixIdx+1]);
    }

    if (xml->ImportFromStream(pStream, false))
    {
      pLogger->Debug("Include ok");
    }
    pStream->Close();

    // restore prefix
    if(newPrefixIdx>0)
    {
      yapt::IContext *pContext = system->GetCurrentContext();
      pContext->SetNamePrefix(prefixCurrent);
    }
  
    ySys->SetActiveDocument(oldDocument);
    delete xml;	
  }	
}

//
// Creates the instance of a plugin object, the instance is always based on a definition object
// which holds the factory for the plugin object
//
IPluginObjectInstance *ExpatXMLParser::CreateObjectInstance(const char *name, const char **atts)
{
  IPluginObjectInstance *pInst_if = NULL;
  int idx = GetAttributeIndex("class", atts);
  if (idx != -1)
  {
    IPluginObjectDefinition *pDef = ySys->GetObjectDefinition(atts[idx+1]);
    if (pDef != NULL)
    {
      pInst_if = pDef->CreateInstance();
      if (pInst_if == NULL)
      {
        yapt::SetYaptLastError(kErrorClass_Import, kError_NoInstance);
        pLogger->Error("Unable to create object instance for '%s' at line %d",atts[idx+1],XML_GetCurrentLineNumber(parser));
      }

//      pInst_if->ExtInitialize();

    } else
    {
      yapt::SetYaptLastError(kErrorClass_Import, kError_ObjectNotFound);
      pLogger->Error("Unable to find object definition for '%s' at line %d",atts[idx+1],XML_GetCurrentLineNumber(parser));
    }


  } else
  {
    yapt::SetYaptLastError(kErrorClass_Import, kError_MissingIdentifier);
    ParseError("Missing Identifier, needs class specification");
  }
  return pInst_if;
}

//
// An execute instance is an object living under the timeline. It controls when, for how long a specific node should
// be rendered (all children are rendered as well).
//
ITimelineExecute *ExpatXMLParser::CreateExecuteInstance(const char *name, const char **atts) {
  ITimeline *pTimeline = ySys->GetActiveDocument()->GetTimeline();
  pLogger->Debug("Create Execute instance, doc=%p, timeline=%p", ySys->GetActiveDocument(), pTimeline);
  ITimelineExecute *pInst = NULL;
  int idxObject = GetAttributeIndex("object",atts);
  int idxStart = GetAttributeIndex("start",atts);
  int idxDuration = GetAttributeIndex("duration",atts);
  if ((idxObject!=-1)&&(idxStart!=-1)&&(idxDuration!=-1)) {
    pInst = pTimeline->AddExecuteObject(atof(atts[idxStart+1]),atof(atts[idxDuration+1]),(char *)atts[idxObject+1]);
  } else {
    yapt::SetYaptLastError(kErrorClass_Import, kError_MissingIdentifier);
    pLogger->Error("Unable to create object instance for '%s' at line %d - one or more attribute(s) failed!",name,XML_GetCurrentLineNumber(parser));
  }
  return pInst;
}

ISignalChannel *ExpatXMLParser::CreateSignalChannelInstance(const char *name, const char **atts) {
  ISignals *signals = ySys->GetActiveDocument()->GetSignals();
  pLogger->Debug("Create signal channel instance, doc=%p, timeline=%p", ySys->GetActiveDocument(), signals);
  ISignalChannel *channel = NULL;
  int idxId = GetAttributeIndex("id",atts);
  int idxName = GetAttributeIndex("name",atts);
  int idxAccuracy = GetAttributeIndex("accuracy",atts);
  if ((idxId!=-1)&&(idxName!=-1)&&(idxAccuracy!=-1)) {
    channel = signals->AddChannel(atoi(atts[idxId+1]),atts[idxName+1],atof(atts[idxAccuracy+1]));
  } else {
    yapt::SetYaptLastError(kErrorClass_Import, kError_MissingIdentifier);
    pLogger->Error("Unable to create signal channel for '%s' at line %d - one or more attribute(s) failed!",name,XML_GetCurrentLineNumber(parser));
  }
  return channel;
}

// callback when expat find the start of an element
typedef enum 
{
  kElementAction_None,
  kElementAction_AddAsResource,
  kElementAction_AddAsRender,
  kElementAction_AddAsResourceContainer,
  kElementAction_AddAsRenderContainer,
  kElementAction_AddToTimeline,
  kElementAction_AddToSignals,
} kElementAction;

void ExpatXMLParser::doStartElement(const char *name, const char **atts)
{
  int i;
  kElementAction action = kElementAction_None;
  kParserState currState = stateStack.top();
  // changes state
  // todo: refactor function, better name needed
  if (!IsElementAllowed(name))
  {
    pLogger->Error("Element '%s' not allowed in state: %d",name, stateStack.top());
    exit(1);
  }

  //for (i = 0; i < depthPtr; i++)
  //  putchar('\t');
  //puts(name);

  IBaseInstance *pInstance = NULL;
  switch(currState)
  {
  case kParserState_Root :
    if (pDocument == NULL)
    {
      pDocument= ySys->CreateNewDocument(true);
    }
    pInstance = dynamic_cast<IBaseInstance *>(pDocument);
    break;
  case kParserState_Doc :
    if (!strcmp(name, kDocument_ResourceTagName)) 
    {
      // we always have one.. replace when supporting multiple
      pInstance = dynamic_cast<IBaseInstance *>(pDocument->GetResources());
    } else if (!strcmp(name, kDocument_RenderTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(pDocument->GetRenderRoot());
    } else if (!strcmp(name, kDocument_TimelineTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(pDocument->GetTimeline());
    } else if (!strcmp(name, kDocument_SignalsTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(pDocument->GetSignals());
    }    
    else if (!strcmp(name,kDocument_IncludeTagName))
    {
      int idx = GetAttributeIndex("url",atts);
      if (idx != -1)
      {
        // Surround the include directive with a meta referer object
        // will not flatten structure when saving!

        IDocNode *pNode = pDocument->AddMetaObject(instanceStack.top());
        pInstance = pNode->GetNodeObject();

        const char *url = atts[idx+1];
        pInstance->AddAttribute("url",url);
        instanceStack.push(pInstance);
        IncludeFromURL(atts[idx+1], atts);
        instanceStack.pop();

        // TODO: Needs to go in to the node tree..
      }

    }
    break;
  case kParserState_Include :
    // no action in this state - empty
    break;
  case kParserState_Resources :
    if (!strcmp(name, kDocument_ObjectTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(CreateObjectInstance(name,atts));
      action = kElementAction_AddAsResource;
    }
    break;
  case kParserState_Signals :
    if (!strcmp(name, kDocument_SignalChannelTagName)) {
      pInstance = dynamic_cast<IBaseInstance *>(CreateSignalChannelInstance(name, atts));
      action = kElementAction_AddToSignals;
    }
    break;
  case kParserState_SignalChannel :
    if (!strcmp(name, kDocument_SignalTagName)) {
      pInstance = instanceStack.top();
      ISignalChannel *channel = dynamic_cast<ISignalChannel *>(pInstance);
      if (channel != NULL) {
        int idxAt = GetAttributeIndex("at", atts);
        int idxValue = GetAttributeIndex("value", atts);
        if ((idxAt != -1) && (idxValue != -1)) {
            ISignal *sig = channel->AddSignal(atof(atts[idxAt+1]), atoi(atts[idxValue+1]));
            pDocument->AddObject(pInstance,dynamic_cast<IBaseInstance *>(sig),kNodeType_ObjectInstance);
        } else {
            pLogger->Error("Unable to create signal at line %d - one or more attribute(s) failed!",XML_GetCurrentLineNumber(parser));
            yapt::SetYaptLastError(kErrorClass_Import, kError_MissingIdentifier);
        }
      } else {
        pLogger->Error("Unable to create signal at line %d - channel is NULL", XML_GetCurrentLineNumber(parser));
      }
    }
    break;
  case kParserState_Timeline :
    if (!strcmp(name, kDocument_ExecuteTagName)) {
      pInstance = dynamic_cast<IBaseInstance *>(CreateExecuteInstance(name, atts));
      action = kElementAction_AddToTimeline;
    }
    break;
  case kParserState_Render:
    if (!strcmp(name, kDocument_ObjectTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(CreateObjectInstance(name,atts));
      action = kElementAction_AddAsRender;
    }
    break;
  case kParserState_Object:	// can be resource - must know
    if (!strcmp(name, kDocument_ObjectTagName))
    {
      pInstance = dynamic_cast<IBaseInstance *>(CreateObjectInstance(name,atts));
      action = kElementAction_AddAsRender;
    } else if (!strcmp(name, kDocument_PropertyTagName))
    {	
      pInstance = instanceStack.top();
      IPluginObjectInstance *pObjectInstance = dynamic_cast<IPluginObjectInstance *>(pInstance);
      int idx = GetAttributeIndex("name",atts);
      if (idx != -1)
      {
        IPluginObject *pExtObject = pObjectInstance->GetExtObject();
        // TODO: need better handling in order to assign attributes
        Property *prop = pObjectInstance->GetProperty(atts[idx+1]);
        if (prop == NULL)
        {
          prop = pObjectInstance->CreateProperty(atts[idx+1],kPropertyType_Unbound,NULL,"");
          if (prop == NULL) {
            // No such property found.. 
            // Create one?
            pLogger->Error("No such property '%s'",atts[idx+1]);
            yapt::SetYaptLastError(kErrorClass_Import, kError_MissingIdentifier);
          }
        }

        pInstance = dynamic_cast<IBaseInstance *>(pObjectInstance->GetPropertyInstance(atts[idx+1]));

        if (prop!=NULL)
        {
          pCdataProperty = prop;
          bCaptureCData = true;
          cdata = "";
        }
      } else
      {
        ParseError("Required attribute 'name' not found");
      }
    }
    break;
  }

  if (pInstance == NULL)
  {
    XML_StopParser(parser, false);
    return;
  }
  // Add attributes to all instances
  if (pInstance != NULL)
  {
    i=0;
    while (atts[i]!=NULL)
    {
      pInstance->AddAttribute(atts[i],atts[i+1]);
      i+=2;
    }

  } 

 
  //
  // Add object at the end of element parsing, after all attributes has been asssigned
  //
  switch (action)
  {
  case kElementAction_AddAsResource :
    pDocument->AddResourceObject(instanceStack.top(),pInstance);
    break;
  case kElementAction_AddAsRender :
    pDocument->AddRenderObject(instanceStack.top(),pInstance);
    break;
  case kElementAction_AddToTimeline :
    pDocument->AddToTimeline(pInstance);
    break;
  case kElementAction_AddToSignals :
    pDocument->AddSignalChannel(dynamic_cast<ISignalChannel *>(pInstance));
    break;
  case kElementAction_None :
    break;
  default:
    pLogger->Warning("Unknown element action '%d', skipping",(int)action);

  }
  instanceStack.push(dynamic_cast<IBaseInstance *>(pInstance));

  depthPtr += 1;
}

// called when expat find the end to an element
void ExpatXMLParser::doEndElement(const char *name)
{
  depthPtr -= 1;
  stateStack.pop();

  IBaseInstance *pTopItem = instanceStack.top();
  instanceStack.pop();

  if (bCaptureCData)
  {
    bCaptureCData = false;
    IPropertyInstance *pInst = dynamic_cast<IPropertyInstance *>(pTopItem);
    pInst->SetValue(cdata.c_str());
    cdata.resize(0);
  }

}
// called during reading of tag data, can be called multiple times
// because data is accumulated.
void ExpatXMLParser::doHandleCharData(const XML_Char *s,int len)
{
  if (bCaptureCData)
  {
    char *buf = (char *)alloca(len+1);	// stack, no need to free
    memcpy(buf, s, len);
    buf[len]='\0';
    cdata.append(buf);
  }
}

void ExpatXMLParser::doHandleComment(const char *data)
{
  IDocNode *pNode = pDocument->AddCommentObject(instanceStack.top());
  IBaseInstance *pInstance = pNode->GetNodeObject();
  ICommentInstance *pComment = dynamic_cast<ICommentInstance *>(pInstance);

  pComment->SetComment((char *)data);
}
// --> Start of XML Parser callback's
// -- not used
static void XMLCALL startCdataSection(void *userData)
{
  printf("-->cdata\n");
}
// -- not used
static void XMLCALL endCdataSection(void *userData)
{
  printf("<--cdata\n");

}
static void XMLCALL characterDataHandler(void *userData,
           const XML_Char *s,
           int len)
{
  ExpatXMLParser *parser = (ExpatXMLParser *)userData;
  parser->doHandleCharData(s, len);
  //	printf("CHAR DATA\n");
}
static void XMLCALL startElement(void *userData, const char *name, const char **atts)
{
  ExpatXMLParser *parser = (ExpatXMLParser *)userData;
  parser->doStartElement(name,atts);
}

static void XMLCALL endElement(void *userData, const char *name)
{
  ExpatXMLParser *parser = (ExpatXMLParser *)userData;
  parser->doEndElement(name);
}

static void XMLCALL commentHandler(void *userData, const char *data) {
  ExpatXMLParser *parser = (ExpatXMLParser *)userData;
  parser->doHandleComment(data);
}

bool ExpatXMLParser::ReadStream(noice::io::IStream *pStream)
{
  bool bRes = true;
  char buf[BUFSIZ];
  int done;

//	pLogger->Debug("Reading XML file: %s",filename);

  parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, (void *)this);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCdataSectionHandler(parser, startCdataSection, endCdataSection);
  XML_SetCharacterDataHandler(parser, characterDataHandler);
  XML_SetCommentHandler(parser, commentHandler);

  instanceStack.push(NULL);
  stateStack.push(kParserState_Root);

  do {
//		int len = (int)fread(buf, 1, sizeof(buf), fInput);
    memset(buf,0,BUFSIZ);
    int len = pStream->Read(buf,BUFSIZ);
    done = len < sizeof(buf);
    if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) 
    {
      ParseError("Syntax error");
      bRes = false;
      done = 1;
    }
  } while (!done);

  XML_ParserFree(parser);

  return bRes;
}


IDocument *ExpatXMLParser::GetDocument()
{
  return pDocument;
}
bool ExpatXMLParser::ImportFromStream(noice::io::IStream *pStream, bool bCreateNewDocument)
{
//	return false;	
  if (!bCreateNewDocument)
  {
    if (pRoot != NULL) {
      pDocument = pRoot->GetDocument();
    } else  {
      pDocument = ySys->GetActiveDocument();
    }
    if (pDocument == NULL) 
    {
      pLogger->Error("No active document, cant import to nothing!");
      return false;
    }

  }
  return ReadStream(pStream);
}

bool ExpatXMLParser::ImportFile(const char *filename, bool bCreateNewDocument)
{
  bool bRes = false;
  if (!bCreateNewDocument)
  {
    pDocument = ySys->GetActiveDocument();
    if (pDocument == NULL) 
    {
      pLogger->Error("No active document, cant import to nothing!");
      return false;
    }
  }
  // Declared in ySystem_Internal
  BasicFileStream *pStream = new BasicFileStream(filename);
  if (pStream)
  {
    if (pStream->Open(kStreamOp_ReadOnly))
    {
      bRes = ReadStream(pStream);
    }
  }
  return bRes;
}


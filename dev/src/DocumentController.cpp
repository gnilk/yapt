/*-------------------------------------------------------------------------
 File    : $Archive: DocumentController.cpp $
 Author  : $Author: Fkling $
 Version : $Revision: 1 $
 Orginal : 2009-10.08, 15:50
 Descr   : Controls the execution of the document
 
 Refactoring of control out of the document
 
 Modified: $Date: $ by $Author: Fkling $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
 - Break up 'RenderNode' to a renderer per node type
 - Replace bUpdate with a proper state
 - Idea: Introduce different document renderes that contain
   rendering logic, we want to keep this logic!
   That way 'force' can be removed, and instead it is
   just another render-logic handler..
 </pre>
 
 
 \History
 - 08.10.09, FKling, Implementation
 - 09.10.09, FKling, Renamed to DocumentController
 - 04.11.09, FKling, Fixed property binding, was broken due to refactoring
 - 01.10.12, FKling, Rewrote initialization, had the logic all mixed up
 
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


DocumentController::DocumentController(IDocument *pDocument)
{
  pLogger = Logger::GetLogger("DocumentController");
  this->pDocument = pDocument;
  this->pSys = yapt::GetYaptSystemInstance();
  renderVars = new RenderVars();
}

DocumentController::~DocumentController()
{
  delete renderVars;
}

//
// non interface functions
//
void DocumentController::SetDocument(IDocument *pDocument)
{
  this->pDocument = pDocument;
}

float DocumentController::GetTimeLineMaxTime() {
  return timelineMaxTime;  
}
//
// Updates the rendering parameters
//
void DocumentController::UpdateRenderVars(double sample_time)
{
  renderVars->IncRenderRef();
  renderVars->SetTime(sample_time);

  // Calculate FPS here
}

//
// Returns the rendering parameters
//
IRenderVars *DocumentController::GetRenderVars()
{
  return dynamic_cast<IRenderVars *>(renderVars);
}


//
// Initializes a node, called directly after the constructor
//
void DocumentController::InitializeNode(IDocNode *node)
{
  //	ISystem *ySys = yapt::GetYaptSystemInstance();
  IBaseInstance *pObject = node->GetNodeObject();
  if (pObject != NULL)
  {
    // pLogger->Debug("Initialize: (%d) %s",pObject->GetInstanceType(),pObject->GetFullyQualifiedName());
    switch(pObject->GetInstanceType())
    {
      case kInstanceType_Object :
      {
        PluginObjectInstance *pInst = dynamic_cast<PluginObjectInstance *>(pObject);


        const char *channelName = pObject->GetAttributeValue("sigchannel");
        if (channelName != NULL) {
            ISignalChannel *channel = pDocument->GetSignals()->GetChannel(channelName);            
            if (channel != NULL) {
              pLogger->Debug("Register '%s' to signal channel %s", pInst->GetInstanceName(), channelName);
              channel->RegisterSinkObject(pInst);
            } else {
              pLogger->Debug("Unable to find channel named '%s' for object '%s'", channelName, pObject->GetAttributeValue("name"));
            }
        }

        pInst->ExtInitialize();
      }
      break;
      case kInstanceType_MetaNode :
      {				
        MetaInstance *pMeta = dynamic_cast<MetaInstance *> (pObject);
        InitializeNode(pMeta->GetDocument()->GetTree());
      }
      break;
    }
  }
  for(int i=0;i<node->GetNumChildren();i++)
  {
    IDocNode *child = node->GetChildAt(i);
    InitializeNode(child);
  }
}

void DocumentController::TraverseNode(IDocumentTraversalSink *sink, IDocNode *node, int depth)
{
  sink->OnNode(node, depth);
  for(int i=0;i<node->GetNumChildren();i++)
  {
    IDocNode *child = node->GetChildAt(i);
    TraverseNode(sink, child, depth+1);
  }
}

void DocumentController::TraverseDocument(IDocumentTraversalSink *sink)
{
  TraverseNode(sink, pDocument->GetTree(), 0);
}


//
// Calls postinitialize, first on all children and on the way back on the node itself
// Called when all sub objects have been created or any property of the parent/children objects has changed
//
bool DocumentController::PostInitializeNode(IDocNode *node) {
  int i,nChildren;
  IBaseInstance *pObject = node->GetNodeObject();

  nChildren = node->GetNumChildren();
  for(i=0;i<nChildren;i++)
  {
    IDocNode *child = node->GetChildAt(i);
    if (!PostInitializeNode(child)) return false;
  }


  if (pObject != NULL) {
    switch(pObject->GetInstanceType()) {
      case kInstanceType_Object : 
      {
        PluginObjectInstance *pInst = dynamic_cast<PluginObjectInstance *>(pObject);
        pInst->ExtPostInitialize();
      }
      break;  
      case kInstanceType_MetaNode :
      {				
        MetaInstance *pMeta = dynamic_cast<MetaInstance *> (pObject);
        PostInitializeNode(pMeta->GetDocument()->GetTree());
      }
      break;
    }
  }
  
  return true;
}
bool DocumentController::BindAllProperties(IDocNode *node) {
  //	ISystem *ySys = yapt::GetYaptSystemInstance();
  IBaseInstance *pObject = node->GetNodeObject();
  if (pObject != NULL)
  {
    switch(pObject->GetInstanceType())
    {
      case kInstanceType_Object :
      {
        PluginObjectInstance *pInst = dynamic_cast<PluginObjectInstance *>(pObject);
        if (!pInst->BindProperties()) {
          return false;
        }
      }
      break;
      case kInstanceType_MetaNode :
      {
        MetaInstance *pMeta = dynamic_cast<MetaInstance *> (pObject);
        BindAllProperties(pMeta->GetDocument()->GetTree());
      }
      break;
    }
  }

  for(int i=0;i<node->GetNumChildren();i++)
  {
    IDocNode *child = node->GetChildAt(i);
    if (!BindAllProperties(child)) return false;
  }
  return true;
}

void DocumentController::SetDirty(IDocNode *pNode)
{
  dirtyNodes.push_back(pNode);
}

bool DocumentController::Initialize() {
  bool result = false;
  InitializeNode(pDocument->GetTree());
  if(BindAllProperties(pDocument->GetTree())) {
    if (PostInitializeNode(pDocument->GetTree())) {
      result = true;
    }
  }
  dirtyNodes.clear();
  
  if (pDocument->HasTimeline()) {
    ITimeline *pTimeline = pDocument->GetTimeline();    
    int n = pTimeline->GetNumExecutors();
  //  pLogger->Debug("RenderTimeLine, t=%f exec=%d, doc=%p",renderVars->GetTime(), n, pDocument);

    float maxTime = 0;
    for(int i=0;i<n;i++) {
      TimelineExecute *pExec = dynamic_cast<TimelineExecute *>(pTimeline->GetExecutorAtIndex(i));
      float tStart = pExec->GetStart();
      float tEnd = tStart + pExec->GetDuration();
      if (tEnd > maxTime) {
        maxTime = tEnd;
      }
    }
    timelineMaxTime = maxTime;
  }
  

  return result;
}

void DocumentController::RenderSignals(double t) {
  ISignals *signals = pDocument->GetSignals();
  int nChannels = signals->GetNumChannels();
  for(int i=0;i<nChannels;i++) {

    ISignalChannel *channel = signals->GetChannel(i);
    ISignal *sig = channel->RaiseNext(t);
    if (sig != NULL) {      
      pLogger->Debug("Signal raised (t=%f) on channel '%s' with value '%d'", t, channel->GetName(), sig->GetValue());
      channel->SendToSinks(sig);
    }
  }
}

void DocumentController::Render(double sample_time) {
  if (dirtyNodes.size() > 0) {
    // we have dirty nodes - poor mans solution - reinitialize the whole tree!
    // This will be costly if the document grows..
    PostInitializeNode(pDocument->GetTree());
    dirtyNodes.clear();
  }

  UpdateRenderVars(sample_time);
  fpsController.Update(sample_time);

  RenderSignals(sample_time);

  if (!pDocument->HasTimeline()) {
    RenderNode(pDocument->GetRenderTree(), false);
  } else {
    RenderTimeline();
  }
}

void DocumentController::RenderTimeline() {
  if (!pDocument->HasTimeline()) return;
  ITimeline *pTimeline = pDocument->GetTimeline();
  int n = pTimeline->GetNumExecutors();
//  pLogger->Debug("RenderTimeLine, t=%f exec=%d, doc=%p",renderVars->GetTime(), n, pDocument);

  for(int i=0;i<n;i++) {
    TimelineExecute *pExec = dynamic_cast<TimelineExecute *>(pTimeline->GetExecutorAtIndex(i));
    if ((pExec != NULL) && (pExec->ShouldRender(renderVars->GetTime()))) {

      IDocNode *pNode = pExec->GetNode();
      if (pNode == NULL) {
        // resolve and cache
        char *simpleName =pExec->GetObjectName();
        // TODO: cache this one - no need to look this up each time!
        pLogger->Debug("RenderTimeLine, t=%f, exec=%s", renderVars->GetTime(), simpleName);
        IBaseInstance *pObject=pDocument->GetObjectFromSimpleName(simpleName);
        pNode=pDocument->FindNode(pObject);

        if (pNode == NULL) {
          pLogger->Error("Document node is NULL for '%s' (%p)",simpleName, pObject);
          exit(1);
        }
        // cache it - this will not change during runtime
        pExec->SetNode(pNode);
      }

      double tStart = pExec->GetStart();
      renderVars->PushLocal(tStart);        
      RenderNode(pNode, true);  // Override timings of object (if any) since controlled by the timeline
      renderVars->PopLocal();
    }
  }
}

void DocumentController::RenderResources()
{
  pLogger->Debug("Render Resources for current Documents");
  RenderResourcesForDocument(pDocument);
  pLogger->Debug("Render Resources for Includes");
  RenderResourcesForIncludes(pDocument->GetTree());
  pLogger->Debug("All resources done!");
}

void DocumentController::RenderResourcesForIncludes(IDocNode *node) {
    int i,nChildren;

    if (node->GetNodeType() == kNodeType_Meta) {
      IBaseInstance *baseObject = node->GetNodeObject();
      IMetaInstance *pMeta = dynamic_cast<IMetaInstance *>(baseObject);
      //printf("Meta node found - processing\n");
      if (pMeta != NULL) {
        IDocument *metaDoc = pMeta->GetDocument();
        RenderResourcesForDocument(metaDoc);
      } else {
        pLogger->Debug("node object is not meta node");
      }
    }

    nChildren = node->GetNumChildren();
    for(i=0;i<nChildren;i++)
    {
      IDocNode *child = node->GetChildAt(i);
      RenderResourcesForIncludes(child);
    }
}

void DocumentController::RenderResourcesForDocument(IDocument *pDocument) {
  IDocNode *pNodeResourceContainer = pDocument->FindNode(dynamic_cast<IBaseInstance *>(pDocument->GetResources()));
  if(pNodeResourceContainer != NULL) {
    RenderNode(pNodeResourceContainer,true);
    pLogger->Debug("Resource container ok");
  } else {
    pLogger->Warning("Resource container not found, skipping");
  }

}

//
// Renders a single node and recursivley all children
// Force is used to override timing information
//
void DocumentController::RenderNode(IDocNode *node, bool bForce)
{
  IBaseInstance *pObject = node->GetNodeObject();
  PluginObjectInstance *pInst = NULL;
  bool bDoChildren = false;	
  bool bPostRender = false;
  
  //if ((pObject != NULL) && (pObject->GetInstanceType() == kInstanceType_Object)) {
  if ((pObject != NULL) && (node->GetNodeType() == kNodeType_ObjectInstance)) {

    pInst = dynamic_cast<PluginObjectInstance *>(pObject);

    if ((pInst->ShouldRender(renderVars)) || (bForce)) {
      //pLogger->Debug("Render: (%d) %s",pObject->GetInstanceType(),pObject->GetFullyQualifiedName());

      if (!bForce) {
        double tStart = pInst->GetStartTime();
        renderVars->PushLocal(tStart);        
      }

      //pLogger->Debug("Render: (%d) %s (tLocal=%f) ref=%d",pObject->GetInstanceType(),pObject->GetFullyQualifiedName(),renderVars->GetLocalTime(), renderVars->GetRenderRef());
      
      // TODO: call system hook handler
      pInst->ExtRender(renderVars);

      if (!bForce) {        
        renderVars->PopLocal();
      }

      // TODO: Read from pInst
      bDoChildren = true;
      bPostRender = true;
    }
  //} else if ((pObject != NULL) && (pObject->GetInstanceType() == kInstanceType_ResourceContainer)) {
  } else if ((pObject != NULL) && (node->GetNodeType() == kNodeType_ResourceContainer)) {
    bDoChildren = true;
  } else {
//    pLogger->Debug("Can't render node of type %d", node->GetNodeType());
  }

  if (bDoChildren) {
    int i,nChildren;

    pObject->GetContext()->PushRenderObject(pObject);
    nChildren = node->GetNumChildren();
    for(i=0;i<nChildren;i++)
    {
      IDocNode *child = node->GetChildAt(i);
      RenderNode(child, bForce);
    }
    pObject->GetContext()->PopRenderObject();

  }


  // TODO: Check if (pInst->RenderAgain()) -> loop

  
  // post render the instance after all children
  if (bPostRender) {
    pInst->ExtPostRender();
  }
}

//
// THIS IS A TEST ON HOW A SEPARATION OF NODE RENDERERS WOULD LOOK LIKE
// CURRENTLY NOT USED...
//
// The idea is that it will become useful when introducing more complex nodes
// like loops and scripting nodes...
//

class NodeRenderer {
public:
  virtual void RenderNode(IDocNode *node, bool force, IDocumentController *controller) = 0;
  virtual void RenderChildren(IDocNode *node, bool force, IDocumentController *controller);
};

class ObjectInstanceRenderer : NodeRenderer {
public:
  virtual void RenderNode(IDocNode *node, bool force, IDocumentController *controller);
};

class ResourceContainerRenderer : NodeRenderer {
  virtual void RenderNode(IDocNode *node, bool force, IDocumentController *controller);
};

void NodeRenderer::RenderChildren(IDocNode *node, bool force, IDocumentController *controller) {
  int i,nChildren;
  IBaseInstance *pObject = node->GetNodeObject();
  pObject->GetContext()->PushRenderObject(pObject);
  nChildren = node->GetNumChildren();
  for(i=0;i<nChildren;i++)
  {
    IDocNode *child = node->GetChildAt(i);
    controller->RenderNode(child, force);
  }
  pObject->GetContext()->PopRenderObject();
}

void ObjectInstanceRenderer::RenderNode(IDocNode *node, bool force, IDocumentController *controller) {
  IBaseInstance *pObject = node->GetNodeObject();
  PluginObjectInstance *pInst = NULL;
  bool bDoChildren = false; 
  bool bPostRender = false;

  if (pObject != NULL) {

    pInst = dynamic_cast<PluginObjectInstance *>(pObject);

    RenderVars *renderVars = dynamic_cast<RenderVars*>(controller->GetRenderVars());
    if ((pInst->ShouldRender(renderVars)) || (force)) {
      //pLogger->Debug("Render: (%d) %s",pObject->GetInstanceType(),pObject->GetFullyQualifiedName());

      if (!force) {
        double tStart = pInst->GetStartTime();
        renderVars->PushLocal(tStart);        
      }

      //pLogger->Debug("Render: (%d) %s (tLocal=%f)",pObject->GetInstanceType(),pObject->GetFullyQualifiedName(),tStart);
      
      // TODO: call system hook handler
      pInst->ExtRender(renderVars);

      if (!force) {        
        renderVars->PopLocal();
      }

      bDoChildren = true;
      bPostRender = true;
    }
  }

  if (bDoChildren) {
    RenderChildren(node, pObject, controller);
  }

  if (bPostRender) {
    pInst->ExtPostRender();
  }
}

void ResourceContainerRenderer::RenderNode(IDocNode *node, bool force, IDocumentController *controller) {
  RenderChildren(node, force, controller);
}
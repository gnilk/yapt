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


//
// Updates the rendering parameters
//
void DocumentController::UpdateRenderVars(double sample_time)
{
  renderVars->IncRenderRef();
  renderVars->SetTime(sample_time);
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
  pLogger->Debug("Initialize: (%d) %s",pObject->GetInstanceType(),pObject->GetFullyQualifiedName());
  if (pObject != NULL)
  {
    pLogger->Debug("Initialize: (%d) %s",pObject->GetInstanceType(),pObject->GetFullyQualifiedName());
    switch(pObject->GetInstanceType())
    {
      case kInstanceType_Object :
      {
        // TODO: Reroute call through instance in order to track states
        PluginObjectInstance *pInst = dynamic_cast<PluginObjectInstance *>(pObject);
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

//
// Calls postinitialize, first on all children and on the way back on the node itself
// Called when all sub objects have been created or any property of the parent/children objects has changed
//
bool DocumentController::PostInitializeNode(IDocNode *node) {
  int i,nChildren;

  IBaseInstance *pObject = node->GetNodeObject();
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


  nChildren = node->GetNumChildren();
  for(i=0;i<nChildren;i++)
  {
    IDocNode *child = node->GetChildAt(i);
    if (!PostInitializeNode(child)) return false;
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
        pLogger->Debug("BindAll for %p\n",pInst);
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

bool DocumentController::Initialize() {

  InitializeNode(pDocument->GetTree());
  if(BindAllProperties(pDocument->GetTree())) {
    if (PostInitializeNode(pDocument->GetTree())) {
      return true;
    }
  }
  return false;

}

void DocumentController::Render(double sample_time)
{
  UpdateRenderVars(sample_time);
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
  for(int i=0;i<n;i++) {
    ITimelineExecute *pExec = dynamic_cast<ITimelineExecute *>(pTimeline->GetExecutorAtIndex(i));
    if ((pExec != NULL) && (pExec->ShouldRender(renderVars->GetTime()))) {
      char *simpleName =pExec->GetObjectName();
      IBaseInstance *pObject=pDocument->GetObjectFromSimpleName(simpleName);
      IDocNode *pNode=pDocument->FindNode(pObject);
      RenderNode(pNode, true);  // Override timings of object (if any) since controlled by the timeline
    }
  }
}

void DocumentController::RenderResources()
{
  // TODO: Solve this with function in Document
  IDocNode *pNodeResourceContainer = pDocument->FindNode(dynamic_cast<IBaseInstance *>(pDocument->GetResources()));
  if(pNodeResourceContainer != NULL)
  {
    RenderNode(pNodeResourceContainer,true);
  }	
}

//
//
// Renders a single node and recursivley all children
// Force is used to override timing information
//
void DocumentController::RenderNode(IDocNode *node, bool bForce)
{
  IBaseInstance *pObject = node->GetNodeObject();
  bool bDoChildren = true;	
  
  if (pObject != NULL)
  {
    switch(pObject->GetInstanceType())
    {
      case kInstanceType_Object :
      {
        PluginObjectInstance *pInst = dynamic_cast<PluginObjectInstance *>(pObject);
        if ((pInst->ShouldRender(renderVars)) || (bForce))
        {
          double tStart = atof(pInst->GetAttributeValue("start"));
          renderVars->PushLocal(tStart);

          // TODO: Need to update bound properties if not rendered

          // TODO: add check for debugging and so forth
          //pLogger->Debug("Render: (%d) %s (tLocal=%f)",pObject->GetInstanceType(),pObject->GetFullyQualifiedName(),tStart);
          
          // TODO: call system hook handler
          pInst->ExtRender(renderVars);
          renderVars->PopLocal();
        } else
        {
          bDoChildren = false;
        }
      }
        break;
    }
  }
  
  int i,nChildren;
  nChildren = node->GetNumChildren();
  for(i=0;i<nChildren;i++)
  {
    IDocNode *child = node->GetChildAt(i);
    RenderNode(child, bForce);
  }
}

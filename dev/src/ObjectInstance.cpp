/*-------------------------------------------------------------------------
File    : $Archive: yObjectDefinition.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : The result of a plugin object instansation is wrapped in this object
by the system.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
- Clean up constructor
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/logger.h"

#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif


using namespace yapt;



// TODO: Refactor, parent not intresting at this level, definition more intresting instead!
PluginObjectInstance::PluginObjectInstance(PluginObjectDefinition *definition) :
BaseInstance(kInstanceType_Object)
{
  pDef = definition;
  extObject = NULL;
  //	this->parent = NULL;
  pDocument = NULL;
  extState = kExtState_None;
  lastRenderRef = 0xffffffff;
  enablePropertyRefRendering = true;  // Default is to render when sourced
  CreateDefaultAttributes();
}

PluginObjectInstance::PluginObjectInstance(IPluginObjectDefinition *definition) :
BaseInstance(kInstanceType_Object)
{
  pDef = (PluginObjectDefinition *)definition;
  extObject = NULL;
  //this->parent = NULL;
  pDocument = NULL;
  extState = kExtState_None;
  lastRenderRef = 0xffffffff;
  enablePropertyRefRendering= true;
  CreateDefaultAttributes();
}

PluginObjectInstance::~PluginObjectInstance()
{
  if (this->extObject != NULL)  {
    delete this->extObject;
  }
  while(!input_properties.empty()) {
    delete input_properties.back();
    input_properties.pop_back();
  }
  while(!output_properties.empty()) {
    delete output_properties.back();
    output_properties.pop_back();
  }
}

void PluginObjectInstance::CreateDefaultAttributes()
{
  AddAttribute("start","0.0");	
  AddAttribute("duration","-1.0");	// infinte
}

void PluginObjectInstance::Dump()
{
  size_t i,n;
  n = GetNumAttributes();
  for (i=0;i<n;i++)
  {
    Logger::GetLogger("Dump")->Debug("A:%s=%s",GetAttribute((int)i)->GetName(), GetAttribute((int)i)->GetValue());
  }
  Logger::GetLogger("Dump")->Debug("Input Properties: %d",GetNumInputProperties());
  Logger::GetLogger("Dump")->Enter();
  for(i=0;i<input_properties.size();i++)
  {
    input_properties[i]->Dump();
  }
  Logger::GetLogger("Dump")->Leave();
  Logger::GetLogger("Dump")->Debug("Output Properties: %d",GetNumOutputProperties());
  Logger::GetLogger("Dump")->Enter();
  pDef->Dump();
  Logger::GetLogger("Dump")->Leave();
  // TODO: Call definition Dump
  // TODO: Call dump on each property
}
PluginObjectDefinition *PluginObjectInstance::GetDefinition()
{
  return pDef;
}
void PluginObjectInstance::SetDefinition(PluginObjectDefinition *pDef)
{
  this->pDef = pDef;
}

//PluginObjectInstance *PluginObjectInstance::GetParent()
//{
//	return this->parent;
//}
//void PluginObjectInstance::SetParent(PluginObjectInstance *newparent)
//{
//	this->parent = newparent;
//}

bool PluginObjectInstance::CreateInstance()
{
  bool res = false;
  if ((pDef != NULL) && (this->extObject == NULL))
  {
    // Only in this state
    if (extState == kExtState_None)
    {
      this->extObject = pDef->CreateExternalInstance();
      if (this->extObject != NULL)
      {
        extState = kExtState_Created;
        res = true;
      }
    }
  }
  return res;
}

IPluginObject *PluginObjectInstance::GetExtObject()
{
  return extObject;
}

void PluginObjectInstance::SetExtObject(IPluginObject *pObject)
{
  this->extObject = pObject;
}

void PluginObjectInstance::SetDocument(IDocument *pDoc)
{
  pDocument = pDoc;
}

bool PluginObjectInstance::IsResource() {
  IDocNode *node = GetDocumentNode();
  while(node != NULL) {
    if (node->GetNodeType() == kNodeType_ResourceContainer) 
      return true;
    node = node->GetParent();
  }
  return false;
}

static std::string GetEquallyDottedString(std::string propRef, std::string fqName) {
  int nDots = 0;
  size_t pos = 0;
  // Counter number '.' in ref
  while(pos != std::string::npos) {
    pos = propRef.find('.',pos);
    nDots++;   
    if (pos == std::string::npos) break;
    pos++;
  }
  pos = std::string::npos;
  while(nDots > 0) {
    pos = fqName.find_last_of('.',pos);   
    nDots--;
    if (nDots > 0) {
      pos--;
    }
  }
  // Skip '.' - we don't want it, want everything to the right side..
  pos++;
  std::string fqLast = fqName.substr(pos);
  return fqLast;

}
//
// must fix this, only search in the direct render tree
//
IPropertyInstance *PluginObjectInstance::FindPropertyInstance(const char *propertyReference, IDocNode *pRootNode)
{
  ILogger *pLogger = Logger::GetLogger("FindPropertyInstance");
  IPropertyInstance *pSource = NULL;
  std::string propRef(propertyReference);
  bool simpleRef = false;
  if (propRef.find('.') == std::string::npos) {
    simpleRef = true;
  }
  int i;
  for (i=0;i<pRootNode->GetNumChildren();i++)
  {
    IDocNode *pChild = pRootNode->GetChildAt(i);
    PluginObjectInstance *pObject = dynamic_cast<PluginObjectInstance *>(pChild->GetNodeObject());
    if (pObject == NULL) {
      continue;
    }
    const char *sName = pObject->GetInstanceName();
    if (simpleRef && !StrConfCaseCmp(propertyReference, pObject->GetInstanceName())) {
      return pObject->GetPropertyInstance(0,true);
    } else {
      // 'Advanced' reference model..            
      // 1) Look for absolute name targetting variables directly

      int nProp = pObject->GetNumOutputProperties();
      for(int j=0;j<nProp;j++) {

        PropertyInstance *pInst = dynamic_cast<PropertyInstance *>(pObject->GetPropertyInstance(j, true));

        std::string qName = GetEquallyDottedString(propRef, pInst->GetFullyQualifiedName());
        if (!qName.compare(propRef)) {
          // found!
          return pObject->GetPropertyInstance(j, true);
        }                
      }
      // 2) Look for object name - targetting first output implicityly
      if (pSource == NULL) {
        std::string qName = GetEquallyDottedString(propRef, std::string(pObject->GetFullyQualifiedName()));
        if (!qName.compare(propRef)) {                
          return pObject->GetPropertyInstance(0,true); 
        }                
      }

    }
  }
  // did not find node, search parent
  if (pSource == NULL)
  {
    if (pRootNode->GetParent() != NULL) {            
      return FindPropertyInstance(propertyReference, pRootNode->GetParent());
    } 
  }
  return NULL;

}

IPropertyInstance *PluginObjectInstance::FindPropertyInstanceFromRoot(const char *propertyReference) {
  // Search from root - must be done outside
  for (size_t i=0;i<GetDocument()->GetTree()->GetNumChildren();i++)
  {
    IDocNode *pChild = GetDocument()->GetTree()->GetChildAt(i);
    IPropertyInstance *pSourceInst = FindPropertyInstance(propertyReference, pChild);
    if (pSourceInst != NULL) return pSourceInst;
  }
  return NULL;
}

bool PluginObjectInstance::IsEqualPropertyTypes(PropertyInstance *p1,  PropertyInstance *p2) {
  return p1->IsEqualType(p2);
}

//
// Property binding is based on the following rule
// 1) Search from parent first, see if there is an object matching the source string
// 2) Search recurisvely from the document root - this will take the resource section first
//
// the matching is done on string basis. The source string is matched against the name of the object.
//
bool PluginObjectInstance::BindProperties()
{
  size_t i;
  for (i=0;i<input_properties.size();i++)
  {
    PropertyInstance *pInput = input_properties[i];
    if (pInput->IsSourced())
    {
      // bind to sibling...
      char *propertyReference = pInput->GetSourceString();
      IPropertyInstance *pSourceInst;
      // search parent
      pSourceInst = FindPropertyInstance(propertyReference, GetDocumentNode()->GetParent());
      // search render root & resources next
      if (pSourceInst == NULL) {       
        Logger::GetLogger("PluginObjectInstance")->Debug("BindProperties, Looking from root for %s",propertyReference);
        pSourceInst = FindPropertyInstanceFromRoot(propertyReference);
      } else {
        // Source found, do type check
        PropertyInstance *pSource = dynamic_cast<PropertyInstance *>(pSourceInst);
        if (IsEqualPropertyTypes(pInput, pSource))
        {
          Logger::GetLogger("PluginObjectInstance")->Debug("BindProperties, Succesfully bound property for %s.%s to %s",GetInstanceName(), pInput->GetName(), pSource->GetFullyQualifiedName());
          Logger::GetLogger("PluginObjectInstance")->Debug("BindProperties, tIn: %s, tOut: %s", pInput->GetTypeName().c_str(), pSource->GetTypeName().c_str());
          pInput->SetSource(pSource);
        } else
        {			
          // type check failed
          Logger::GetLogger("PluginObjectInstance")->Warning("BindProperties, type mismatch (%s, %s) failed to bind property for %s.%s to %s",
            pInput->GetTypeName().c_str(), pSource->GetTypeName().c_str(),
            GetDefinition()->GetDescription(), pInput->GetName(), propertyReference);
          SetYaptLastError(kErrorClass_ObjectDefinition, kError_PropertyBindFailed);
          return false;
        }            
      }

    }
  } // for
  return true;
}

// - protected
bool PluginObjectInstance::IsPropertyInstanceOutput(PropertyInstance *pInst) 
{
  // check if the instance pointer is part of the output list
  for(int i=0;i<output_properties.size();i++) {
    if (output_properties[i] == pInst) return true;
  }
  return false;
}

// - protected
PropertyInstance *PluginObjectInstance::GetPropertyInstance(std::vector<PropertyInstance *> *pList, const char *szName)
{
  std::vector<PropertyInstance *>::iterator it;

  // TODO: Translate incoming name!

  for (it=pList->begin(); it!=pList->end();it++)
  {
    PropertyInstance *prop = (PropertyInstance *)(*it);
    //		if (!strcmp(prop->GetName(), szName)) 
    if (!StrConfCaseCmp(prop->GetName(), szName)) 
    {
      return prop;
    }
  }
  return NULL;
}

// - protected
PropertyInstance *PluginObjectInstance::GetPropertyInstance(std::vector<PropertyInstance *> *pList, int index)
{
  if (index < (int)pList->size())
  {
    return pList->at(index);
  } else {
    Logger::GetLogger("PluginObjectInstance")->Debug("GetPropertyInstance, index '%d' out of bounds (%d)",index, pList->size());
  }
  return NULL;
}

IPropertyInstance *PluginObjectInstance::GetPropertyInstance(const char *szName, bool bOutput)
{
  PropertyInstance *result;
  if (!bOutput)
  {
    result = GetPropertyInstance(&input_properties, szName);
  } else
  {
    result = GetPropertyInstance(&output_properties, szName);
  }
  return dynamic_cast<IPropertyInstance *>(result);
}
// used when sourcing the default property (first created)
IPropertyInstance *PluginObjectInstance::GetPropertyInstance(int index, bool bOutput)
{
  PropertyInstance *result;
  if (!bOutput)
  {
    result = GetPropertyInstance(&input_properties, index);
  } else
  {
    result = GetPropertyInstance(&output_properties, index);
  }
  return dynamic_cast<IPropertyInstance *>(result);
}


void PluginObjectInstance::AddPropertyInstance(PropertyInstance *property, bool bOutput)
{
  property->SetObjectInstance(this);

  if (!bOutput)
  {
    input_properties.push_back(property);
    // TODO: Register this one with the document		
  } else
  {
    output_properties.push_back(property);
    // Don't register output properties
  }
}
// -- determines if this object should render or not..
bool PluginObjectInstance::ShouldRender(RenderVars *pRenderVars)
{
  bool bRes = false;
  double glbTime = pRenderVars->GetTime();
  double start = atof(GetAttributeValue("start"));
  double duration = atof(GetAttributeValue("duration"));

  if (duration < 0) duration = 1.0+glbTime;	// does not matter but makes the rest of logic pass
  if ((glbTime >= start) && (glbTime < (start+duration))) 
  {
    bRes = true;
  }

  return bRes;
}
//
// -- common attribute functions
//
double PluginObjectInstance::GetStartTime()
{
  // TODO: Optimize, cache value (skip conversion)
  return atof(GetAttributeValue("Start","0"));
}
void PluginObjectInstance::SetStartTime(double time_sec)
{
  char tmp[64];
  snprintf(tmp,64,"%f",time_sec);
  AddAttribute("Start",tmp);
}
double PluginObjectInstance::GetDuration()
{
  return atof(GetAttributeValue("Duration","-1"));
}
void PluginObjectInstance::SetDuration(double durtaion_sec)
{
  char tmp[64];
  snprintf(tmp,64,"%f",durtaion_sec);
  AddAttribute("Duration",tmp);
}
const char *PluginObjectInstance::GetClassName()
{
  return GetAttributeValue("Class","noclass");
}
const char *PluginObjectInstance::GetInstanceName()
{
  return GetAttributeValue("Name","noname");
}
void PluginObjectInstance::SetInstanceName(const char *sNewName)
{
  AddAttribute("Name",sNewName);
}

// -- event from BaseInstance
void PluginObjectInstance::OnAttributeChanged(Attribute *pAttribute)
{
  // support name-prefix here
  const char *name = pAttribute->GetName();
  if (!StrConfCaseCmp(name,"Name"))
  {
    char tmp[128];
    // have prefix? use it..
    if (strcmp(pContext->GetNamePrefix(tmp,128),""))
    {
      pContext->CreatePrefixName(pAttribute->GetValue(),tmp, 128);
      Logger::GetLogger("PluginObjectInstance")->Debug("Prefixing name: '%s' was '%s'",tmp, pAttribute->GetValue());
      pAttribute->SetValue(tmp);
    }
  }
  // This attribute disables/enables rendering of property dependencies
  // good for feedback scenarios..
  if (!StrConfCaseCmp(name,"allowRefUpdate")) {
    if (!StrConfCaseCmp(name,"true")){
      enablePropertyRefRendering = true;
    } else {
      enablePropertyRefRendering = false;
    }
  }
}


// -- interface helper
Property *PluginObjectInstance::CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription, bool bOutput)
{
  Property *result = NULL;
  PropertyInstance *prop = NULL;
  result = GetProperty(sName, bOutput);

  //Logger::GetLogger("PluginObjectInstance")->Debug("CreateProperty: '%s' output %s, found %s",sName, bOutput?"yes":"no", result?"yes":"no");

  bool hasUnboundValue = false;
  if (bOutput == false) {
    if ((result != NULL) && (result->type == kPropertyType_Unbound)) {
      prop = dynamic_cast<PropertyInstance *>(GetPropertyInstance(sName));
      result->type = type;
      prop->SetDescription(sDescription);
      // we should have an unbound value
      hasUnboundValue = prop->GetUnboundRawValue()?true:false;
    }  else if (result != NULL) {
      // recreating input property
      return result;
    }
  }

  // recreate output??
  if ((result != NULL) && (bOutput == true)) {
    return result;
  }

  if (prop == NULL) {
    // Create instance object and add node in document
    prop = new PropertyInstance(sName, type, sDescription);
    if (pDocument) {
      pDocument->AddObject(dynamic_cast<IBaseInstance *>(this),dynamic_cast<IBaseInstance *>(prop),kNodeType_PropertyInstance);            
    }
    AddPropertyInstance(prop, bOutput);        
  }
  // Initial value is NULL during early binding of variables - creating from XML stream reading
  if (sInitialValue != NULL) {
    if (!hasUnboundValue) {
      prop->SetValue(sInitialValue);
    } else {
      prop->SetValue(prop->GetUnboundRawValue());
    }
  }
  // Fetch the actual property and return it
  result = prop->GetProperty();
  return result;

}

// -- interface
Property *PluginObjectInstance::CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription)
{
  return CreateProperty(sName, type, sInitialValue, sDescription, false);
}

Property *PluginObjectInstance::CreateOutputProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription)
{
  return CreateProperty(sName, type, sInitialValue, sDescription, true);
}

void PluginObjectInstance::SetPropertyHint(const char *sName, kPropertyHint hint) 
{
  PropertyInstance *prop = dynamic_cast<PropertyInstance *>(GetPropertyInstance(sName));
  if (prop == NULL) {
      Logger::GetLogger("PluginObjectInstance")->Debug("SetPropertyHint, no property named '%s' was found",sName);
      return;
  }
  prop->SetPropertyHint(hint);
}

kPropertyHint PluginObjectInstance::GetPropertyHint(const char *sName)
{
  PropertyInstance *prop = dynamic_cast<PropertyInstance *>(GetPropertyInstance(sName));
  if (prop == NULL) {
      Logger::GetLogger("PluginObjectInstance")->Debug("GetPropertyHint, no property named '%s' was found",sName);
      return kPropertyHint_None;
  }
  return prop->GetPropertyHint();

}

Property *PluginObjectInstance::GetProperty(const char *name, bool isOutput)
{
  Property *prop = NULL;
  PropertyInstance *pInst;

  if (isOutput) {
    pInst = GetPropertyInstance(&output_properties, name);    
  } else {
    pInst= GetPropertyInstance(&input_properties, name);
  }

  if (pInst != NULL)
  {
    prop = pInst->GetProperty();
  }

  return prop;

}


Property *PluginObjectInstance::GetProperty(const char *name)
{
  Property *prop = NULL;
  PropertyInstance *pInst;

  pInst= GetPropertyInstance(&input_properties, name);
  if (!pInst)
  {
    pInst = GetPropertyInstance(&output_properties, name);
  }
  if (pInst != NULL)
  {
    prop = pInst->GetProperty();
  }

  return prop;
}

void PluginObjectInstance::SetPropertyValue(Property *prop, const char *value)
{
  IPropertyInstance *pInst = Lookup::GetPropertyInstance(prop);
  if (pInst != NULL)
  {
    pInst->SetValue(value);
  }
}

IPropertyInstance *PluginObjectInstance::GetPropertyInstance(const char *name)
{
  IPropertyInstance *pInst = GetPropertyInstance(name, false);
  return dynamic_cast<IPropertyInstance *>(pInst);
}

int PluginObjectInstance::GetNumInputProperties()
{
  return (int)input_properties.size();
}
int PluginObjectInstance::GetNumOutputProperties()
{
  return (int)output_properties.size();
}
Property *PluginObjectInstance::GetInputPropertyAt(int index)
{
  Property *result = NULL;
  if ((index >= 0) && (index < (int)input_properties.size())) 
  {
    result = input_properties[index]->GetProperty();
  }
  return result;
}
Property *PluginObjectInstance::GetOutputPropertyAt(int index)
{
  Property *result = NULL;
  if ((index >= 0) && (index < (int)output_properties.size())) 
  {
    result = output_properties[index]->GetProperty();
  }
  return result;
}

IDocument *PluginObjectInstance::GetDocument()
{
  return dynamic_cast<IDocument *>(pDocument);
}
IDocNode *PluginObjectInstance::GetDocumentNode()
{
  return Lookup::GetNodeForExtInst(extObject);
}


void PluginObjectInstance::ExtInitialize()
{
  if (extState == kExtState_Created)
  {
    extObject->Initialize(yapt::GetYaptSystemInstance(), dynamic_cast<IPluginObjectInstance *>(this));
    extState = kExtState_Initialized;
  } else {
    Logger::GetLogger("PluginObjectInstance")->Error("ExtInitialize, wrong state (%d) expected=%d",extState, kExtState_Created);    
  }
}

void PluginObjectInstance::ExtPostInitialize()
{
//  Logger::GetLogger("PluginObjectInstance")->Debug("ExtPostInitialize, state=%d",extState);
  if (extState == kExtState_Initialized)
  {
    extObject->PostInitialize(yapt::GetYaptSystemInstance(), dynamic_cast<IPluginObjectInstance *>(this));
  } else {
    Logger::GetLogger("PluginObjectInstance")->Error("ExtPostInitialize, not Initialized!! (state=%d)",extState);    
  }
}

bool PluginObjectInstance::IsPropertyRefRendering(){
  return enablePropertyRefRendering;
}
void PluginObjectInstance::SetPropertyRefRendering(bool _enablePropertyRefRendering) {
  enablePropertyRefRendering = _enablePropertyRefRendering;
}

bool PluginObjectInstance::IsDirty() {
  return dirtyFlag;
}
void PluginObjectInstance::SetDirty(bool dirty) {
  dirtyFlag = dirty;
}

void PluginObjectInstance::RenderPropertyDependencies(RenderVars *pRenderVars) {
  // Update dependencies
  for(int i=0;i<input_properties.size();i++) {
    PropertyInstance *pInst = input_properties[i];

    if (pInst->IsSourced()) {
      PropertyInstance *pSourceInst = dynamic_cast<PropertyInstance *>(pInst->GetSource());
      PluginObjectInstance *pSourceObject = dynamic_cast<PluginObjectInstance *>(pSourceInst->GetPluginObjectInstance());
      if (pSourceObject->IsResource()) return;
      if (pSourceObject->IsPropertyRefRendering()) {
        pSourceObject->ExtRender(pRenderVars); 
      }
    }
  }
}

// TODO: Move to controller [external]
void PluginObjectInstance::ExtRender(RenderVars *pRenderVars)
{    
  // Only do this when we have been initialized and if not already rendered
  // Multiple calls can be done when several other objects refer to the properties of this one
//  Logger::GetLogger("PluginObjectInstance")->Debug("ExtRender, state=%d, lastRenderRef=%d (%d)\n",extState, lastRenderRef, pRenderVars->GetRenderRef());

  if ((extState == kExtState_Initialized) && (lastRenderRef != pRenderVars->GetRenderRef()))
  {        

//    Logger::GetLogger("PluginObjectInstance")->Debug("ExtRender, render property dependencies");
    RenderPropertyDependencies(pRenderVars);
    //Logger::GetLogger("PluginObjectInstance")->Debug("ExtRender, calling ext object render: %p",extObject);
    extObject->Render(pRenderVars->GetTime(), dynamic_cast<IPluginObjectInstance *>(this));
    lastRenderRef = pRenderVars->GetRenderRef();
    // Set this to dirty in order to pass call's through the post-renderer
    SetDirty(true);
  }
}
void PluginObjectInstance::ExtPostRender()
{
  if ((extState == kExtState_Initialized) && (IsDirty()))
  {
    extObject->PostRender(0.0, dynamic_cast<IPluginObjectInstance *>(this));
    SetDirty(false);
  }
}



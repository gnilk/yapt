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
	CreateDefaultAttributes();
}

PluginObjectInstance::~PluginObjectInstance()
{
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

//
// must fix this, only search in the direct render tree
//
IPropertyInstance *PluginObjectInstance::FindPropertyInstance(const char *propertyReference, IDocNode *pRootNode)
{
	IPropertyInstance *pSource = NULL;
	int i;
	for (i=0;i<pRootNode->GetNumChildren();i++)
	{
		IDocNode *pChild = pRootNode->GetChildAt(i);
		PluginObjectInstance *pObject = (PluginObjectInstance *)pChild->GetNodeObject();
		// TODO: Verify binding
		const char *sName = pObject->GetAttributeValue("name"); 

		if (!StrConfCaseCmp(sName,propertyReference))
		{
			pSource = pObject->GetPropertyInstance(0,true);
			break;
		}
	}
	// did not find node, search children!
	if (pSource == NULL)
	{
		for (i=0;i<pRootNode->GetNumChildren();i++)
		{
			IDocNode *pChild = pRootNode->GetChildAt(i);
			if ((pChild->GetNodeType() == kNodeType_ObjectInstance) ||
			(pChild->GetNodeType() == kNodeType_ResourceContainer) ||
			(pChild->GetNodeType() == kNodeType_RenderNode))
			{
				pSource = FindPropertyInstance(propertyReference, pChild);
				if (pSource != NULL) 
				{
					break;
				}
			}
			
		}
	}
	return pSource;
	
}

// Todo: this requires a more advanced lookup parser
void PluginObjectInstance::BindProperties()
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
			if (pSourceInst == NULL) pSourceInst = FindPropertyInstance(propertyReference, GetDocument()->GetTree());

			if (pSourceInst != NULL)
			{
				PropertyInstance *pSource = dynamic_cast<PropertyInstance *>(pSourceInst);
				Logger::GetLogger("PluginObjectInstance")->Debug("BindProperties, Succesfully bound property for %s.%s to %s.output[0]",GetInstanceName(), pInput->GetName(), propertyReference);
				pInput->SetSource(pSource);
			} else
			{			
				Logger::GetLogger("PluginObjectInstance")->Warning("BindProperties, failed to bind property for %s.%s to %s.output[0]",GetDefinition()->GetDescription(), pInput->GetName(), propertyReference);
			}

		}
	}
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
bool PluginObjectInstance::ShouldRender(double glbTime)
{
	bool bRes = false;
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
}


// -- interface helper
Property *PluginObjectInstance::CreateProperty(const char *sName, kPropertyType type, const char *sInitialValue, const char *sDescription, bool bOutput)
{
	Property *result = NULL;

	// Create the instance object
	PropertyInstance *prop = new PropertyInstance(sName, type, sDescription);
	prop->SetValue(sInitialValue);

	AddPropertyInstance(prop, bOutput);

	// Create and add node in document
	//IDocument *pDocument = yapt::GetYaptSystemInstance()->GetActiveDocument();
	pDocument->AddObject(dynamic_cast<IBaseInstance *>(this),dynamic_cast<IBaseInstance *>(prop),kNodeType_PropertyInstance);

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
	}
}

void PluginObjectInstance::ExtPostInitialize()
{
	if (extState == kExtState_Initialized)
	{
		extObject->PostInitialize(yapt::GetYaptSystemInstance(), dynamic_cast<IPluginObjectInstance *>(this));
	}
}

// TODO: Move to controller [external]
void PluginObjectInstance::ExtRender(RenderVars *pRenderVars)
{
	// Figure out time through context object, context should be assign to each instance

	// Only do this when we have been initialized
	if (extState == kExtState_Initialized)
	{
	
		extObject->Render(pRenderVars->GetTime(), dynamic_cast<IPluginObjectInstance *>(this));
	}
}
void PluginObjectInstance::ExtPostRender()
{
	if (extState == kExtState_Initialized)
	{
		extObject->PostRender(0.0, dynamic_cast<IPluginObjectInstance *>(this));
	}
}


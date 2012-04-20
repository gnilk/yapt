/*
 *  yObject.cpp
 *  yapt2
 *
 *  Created by Fredrik Kling on 8/23/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "yapt/logger.h"

#include "yapt/yObject.h"
#include "yapt/ySystem.h"

#include "yapt/ySystem_internal.h"

using namespace yapt;


//
// Object definition constructor, takes factory and a description.
//
PluginObjectDefinition::PluginObjectDefinition(IPluginObjectFactory *factory, const char *description) :
	BaseInstance(kInstanceType_ObjectDefinition)
{
	//TODO: assert factory and description

	this->factory = factory;
	this->description = strdup(description);
	ParseDescriptionString();
}
PluginObjectDefinition::~PluginObjectDefinition()
{
}
// -- protected helper
void PluginObjectDefinition::ParseDescriptionString()
{
	// ease up testing
	if (description[0]=='@')
	{
		AddAttribute(kObjectAttribute_Identifier,&description[1]);	
	} else
	{
		// do actual parsing here, split on ';' and than each substring on '='
	}
}

void PluginObjectDefinition::Dump()
{
	Logger::GetLogger("Dump")->Debug("Definition: %s",description);
}


IPluginObjectFactory *PluginObjectDefinition::GetFactory()
{
	return this->factory;
}

IPluginObject *PluginObjectDefinition::CreateInstance(ISystem *ySys, YHANDLE handle)
{
	IPluginObject *result = NULL;
	if(factory != NULL)
	{
		// TODO: Should not use description, should use identifier
		const char *extid  = GetAttributeValue(kObjectAttribute_Identifier);
		result = factory->CreateObject(ySys, handle, extid);
	}
	return result;
}

const char *PluginObjectDefinition::GetDescription()
{
	return (const char *)this->description;
}

// -- Object Instance
// TODO: Refactor, parent not intresting at this level, definition more intresting instead!
PluginObjectInstance::PluginObjectInstance(PluginObjectDefinition *definition) :
	BaseInstance(kInstanceType_Object)
{
	pDef = definition;
	extObject = NULL;
	this->parent = NULL;
	pDocument = NULL;
}
PluginObjectInstance::~PluginObjectInstance()
{
}

void PluginObjectInstance::Dump()
{
	Logger::GetLogger("Dump")->Debug("Input Properties: %d",GetNumInputProperties());
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

PluginObjectInstance *PluginObjectInstance::GetParent()
{
	return this->parent;
}
void PluginObjectInstance::SetParent(PluginObjectInstance *newparent)
{
	this->parent = newparent;
}

bool PluginObjectInstance::CreateInstance(ISystem *ySys, YHANDLE id)
{
	bool res = false;
	if ((pDef != NULL) && (this->extObject == NULL))
	{
		this->extObject = pDef->CreateInstance(ySys, id);
		if (this->extObject != NULL)
			res = true;
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
void PluginObjectInstance::SetDocument(Document *pDoc)
{
	pDocument = pDoc;
}
Document *PluginObjectInstance::GetDocument()
{
	return pDocument;
}

PropertyInstance *PluginObjectInstance::FindProperty(char *propertyReference)
{
	PropertyInstance *pSource = NULL;
	IDocNode *pNode = pDocument->GetRenderRoot();
	for (int i=0;i<pNode->GetNumChildren();i++)
	{
		IDocNode *pChild = pNode->GetChildAt(i);
		PluginObjectInstance *pObject = (PluginObjectInstance *)pChild->GetNodeObject();
		const char *szDescription = pObject->GetDefinition()->GetDescription();
		// lousy
		if (!strcmp(szDescription, propertyReference))
		{
			pSource = pObject->GetProperty(0,true);
			break;
		}
	}
	return pSource;
	
}

// Todo: this requires a more advanced lookup parser
void PluginObjectInstance::BindProperties()
{
	int i;
	for (i=0;i<input_properties.size();i++)
	{
		PropertyInstance *pInput = input_properties[i];
		
		if (pInput->IsSourced())
		{
			// bind to sibling...
			char *propertyReference = pInput->GetSourceString();
			PropertyInstance *pSource = FindProperty(propertyReference);
			if (pSource != NULL)
			{
				Logger::GetLogger("PluginObjectInstance")->Debug("BindProperties, Succesfully bound property for %s.%s to %s.output[0]",GetDefinition()->GetDescription(), pInput->GetName(), propertyReference);
				pInput->SetSource(pSource);
			}

		}
	}
}


// - protected
PropertyInstance *PluginObjectInstance::GetPropertyInstance(std::vector<PropertyInstance *> *pList, char *szName)
{
	std::vector<PropertyInstance *>::iterator it;
	
	for (it=pList->begin(); it!=pList->end();it++)
	{
		PropertyInstance *prop = (PropertyInstance *)(*it);
		if (!strcmp(prop->GetName(), szName)) 
		{
			return prop;
		}
	}
	return NULL;
}

// - protected
PropertyInstance *PluginObjectInstance::GetPropertyInstance(std::vector<PropertyInstance *> *pList, int index)
{
	if (index < pList->size())
	{
		return pList->at(index);
	}
	return NULL;
}

PropertyInstance *PluginObjectInstance::GetProperty(Property *property)
{
	PropertyInstance *pInst = (PropertyInstance *)property->internal_id;
	return pInst;
}

PropertyInstance *PluginObjectInstance::GetProperty(char *szName, bool bOutput)
{
	PropertyInstance *result;
	if (!bOutput)
	{
		result = GetPropertyInstance(&input_properties, szName);
	} else
	{
		result = GetPropertyInstance(&output_properties, szName);
	}
	return result;
}
// used when sourcing the default property (first created)
PropertyInstance *PluginObjectInstance::GetProperty(int index, bool bOutput)
{
	PropertyInstance *result;
	if (!bOutput)
	{
		result = GetPropertyInstance(&input_properties, index);
	} else
	{
		result = GetPropertyInstance(&output_properties, index);
	}
	return result;
}


void PluginObjectInstance::AddProperty(PropertyInstance *property, bool bOutput)
{
	property->SetObjectInstance(this);
	
	if (!bOutput)
	{
		input_properties.push_back(property);
	} else
	{
		output_properties.push_back(property);
	}
}
int PluginObjectInstance::GetNumInputProperties()
{
	return input_properties.size();
}
int PluginObjectInstance::GetNumOutputProperties()
{
	return output_properties.size();
}



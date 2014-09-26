/*-------------------------------------------------------------------------
File    : $Archive: yProperty.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Holds the property instances and the actual property, a property instance
          gets created as part of plugin object initialization.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Support for 2D positional type (vector2d ?) tuple_float, tuple_int ?
  ! enumerations, syntax: "enum={value1,value2,...,value_n}" the string "value1" will 
	be translated to the integer 0 and set to the property integer it is completely 
	possible to source from an enum, but not adviced to source to an enum.
</pre>


\History
- 17.09.09, FKling, Implementation
- 24.09.09, FKling, Enum implementation
- 04.10.09, FKling, Int/Float tuple and quat support, found errors in Set/Get value
- 04.11.09, FKling, Fixed source support, had become broken

---------------------------------------------------------------------------*/
#include "yapt/logger.h"

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"
#include "yapt/StringUtil.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;


static int StrToken(const char *sInput, int iOffset, char *dest, int nMax);

// -- property instance

PropertyInstance::PropertyInstance(const char *szName, kPropertyType type, const char *sDescription) :
	BaseInstance(kInstanceType_Property)
{
	property = new Property();
	property->v = new PropertyValue();
	property->type = type;
	// zero out the values
	memset(property->v, 0, sizeof(PropertyValue));

	this->sDescription = strdup(sDescription);
	this->textDescription = NULL;
	this->userTypeName = NULL;
	ParseDescriptionString();
	// Distinguish between them, in order to easily source/unsource the same property
	v_unsourced = NULL;
	unboundRawValue = NULL;
	sourcedProperty = NULL;
	sourceString = NULL;
	isSourced = false;
	iSourceReferences = 0;

	pLogger = Logger::GetLogger("PropertyInstance");

	// add the name, must be part of base instance 
	// ---
	AddAttribute("name",szName);

	// register all properties, allow for fast lookup's
	Lookup::RegisterPropInst(property, dynamic_cast<IPropertyInstance *>(this));
}

PropertyInstance::~PropertyInstance()
{
	free(this->sDescription);
	if (!isSourced)
	{
		delete property->v;
	}
	if (sourceString != NULL)
	{
		free (sourceString);
	}
	delete property;
}
void PropertyInstance::Dump()
{
	char tmp[256];
	int i,n;
	n = GetNumAttributes();
	// Dump uses other logger
	Logger::GetLogger("Dump")->Debug("V:%s",GetValue(tmp,265));
	for (i=0;i<n;i++)
	{
		Logger::GetLogger("Dump")->Debug("A:%s=%s",GetAttribute(i)->GetName(), GetAttribute(i)->GetValue());
	}
}

Property *PropertyInstance::GetProperty()
{
	return this->property;
}

void PropertyInstance::SetProperty(Property *property)
{
	this->property = property;
}

void PropertyInstance::SetPropertyHint(kPropertyHint hint)
{
	this->propertyHint = hint;
}

kPropertyHint PropertyInstance::GetPropertyHint()
{
	return this->propertyHint;
}


bool PropertyInstance::IsOutputProperty() 
{
	return this->objectInstance->IsPropertyInstanceOutput(this);
}

bool PropertyInstance::IsSourced()
{
	bool bRes = false;
	// source string is null, check if we have been sourced
	if (this->sourceString == NULL)
	{
		if (this->GetAttributeValue("source",NULL) != NULL)
		{
			this->sourceString = strdup(this->GetAttributeValue("source",NULL));
			bRes = true;
		}
	} else
	{
		bRes = true;	// we are sourced...
	}
	return bRes;
}

void PropertyInstance::OnAttributeChanged(Attribute *pAttribute)
{
	// support name-prefix here
	const char *name = pAttribute->GetName();
	if (!StrConfCaseCmp(name,"Source"))
	{
		char tmp[128];
		// have prefix? use it..
		if (strcmp(pContext->GetNamePrefix(tmp,128),""))
		{
			pContext->CreatePrefixName(pAttribute->GetValue(),tmp, 128);
			Logger::GetLogger("PropertyInstance")->Debug("Prefixing name: '%s' was '%s'",tmp, pAttribute->GetValue());
			pAttribute->SetValue(tmp);
		}
	}
}

void PropertyInstance::BindProperty(char *propertyReference)
{
	sourceString = strdup(propertyReference);
}
char *PropertyInstance::GetSourceString()
{
	return sourceString;
}
int PropertyInstance::IncSourceRef()
{
	return ++iSourceReferences;
}
int PropertyInstance::DecSourceRef()
{
	if (iSourceReferences > 0)
	{
		iSourceReferences--;
	}
	return iSourceReferences;
}

// Set the property sourcing, supply NULL to break an already sourced property
void PropertyInstance::SetSource(PropertyInstance *pSource) {
	// Already sourced? Decrease previous target reference, don't backup the value
	if (isSourced && sourcedProperty!=NULL) {
		sourcedProperty->DecSourceRef();
	}

	Logger::GetLogger("PropertyInstance")->Debug("SetSource for '%s' to '%s'",
		this->GetFullyQualifiedName(),
		pSource->GetFullyQualifiedName());

	// if NULL then we should break sourceing and go back to old value
	if(pSource != NULL)	{
		if (!isSourced)
		{
			// first time we are sourced, save the orginal value
			v_unsourced = property->v;
			isSourced = true;	// yes, we are!
		}
		// fetch sourced property value and increase sourced property reference count
		property->v = pSource->GetProperty()->v;		
		pSource->IncSourceRef();
	}
	else // pSouce == NULL
	{
		// Restore previous value - if we were sourced
		if (isSourced) {
			property->v = v_unsourced;			
		}
		// reset
		isSourced = false;
		v_unsourced = NULL;
	}
	sourcedProperty = pSource;
}

IPropertyInstance *PropertyInstance::GetSource() {
	return sourcedProperty;
}

kPropertyType PropertyInstance::GetPropertyType() {
  // No need to check source type (if sourced) can bind to different type than self..
  if (GetSource() != NULL) {
    return GetSource()->GetPropertyType();
  }
	return property->type;
}

const char *PropertyInstance::GetName() {
	return GetAttributeValue("name");
}

const char *PropertyInstance::GetDescription() {
	return sDescription;
}

void PropertyInstance::SetDescription(const char *strDesc) {
  if (sDescription==NULL) free(sDescription);
  this->sDescription = strdup(strDesc);
}

void PropertyInstance::SetObjectInstance(PluginObjectInstance *owner) {
	this->objectInstance = owner;
}

PluginObjectInstance *PropertyInstance::GetPluginObjectInstance() {
	return this->objectInstance;	
}

IBaseInstance *PropertyInstance::GetObjectInstance() {
	return dynamic_cast<IBaseInstance *>(this->objectInstance);
}

// The description string can be
//
// ';' and '=' reserved 
//
// ex1;	plain text description of the property
// ex1: type=MyType;description=holds my type
//
void PropertyInstance::ParseDescriptionString() {
	std::vector<std::string> params;

	// if no arguments - assume plain text description
	if (std::string(this->sDescription).find(';') == std::string::npos) {
		this->textDescription = this->sDescription;	// same
		return;
	}

	StrSplit(params, this->sDescription, ';');

	for(int i=0;i<params.size();i++) {
		std::vector<std::string> tokens;
		StrSplit(tokens, params[i].c_str(), '=');
		if (tokens.size() != 2) {
			pLogger->Error("Description string parse error on '%s'",params[i].c_str());
			continue;
		}

		switch(StrConfCaseCmp(tokens[0].c_str(), "type description desc")) {
			case 1 : // 'type' - user type name
				this->userTypeName = strdup(tokens[1].c_str());
				break;
			case 2 :
			case 3 :
				// plain text description for property
				this->textDescription = strdup(tokens[1].c_str());
				break;
			default:
				pLogger->Debug("Unknown token in type descriptor '%s'", tokens[0].c_str());
				break;
		}
	}

}

bool PropertyInstance::IsEqualType(PropertyInstance *pOther) {
	// not user ptr, are they equal??
	if (GetPropertyType() != kPropertyType_UserPtr) {
		return (GetPropertyType() == pOther->GetPropertyType());
	}
		
	// They are both user ptr, check if type names are same
	if ((GetPropertyType() == pOther->GetPropertyType()) && (GetPropertyType() == kPropertyType_UserPtr)) {
		// should work for 'empty' type names as well
		// todo: add config option to relax this
		return (GetTypeName() == pOther->GetTypeName());
	}

	return false;
}

std::string PropertyInstance::GetTypeName() {
	char tmp[256];
	GetPropertyTypeName(tmp, 256);
	return std::string(tmp);
}

char *PropertyInstance::GetPropertyTypeName(char *sDest, int maxLen) {
    switch(GetPropertyType()) {
		case kPropertyType_Float:
            snprintf(sDest, maxLen, "float");
			break;
		case kPropertyType_FloatTuple :
            snprintf(sDest, maxLen, "vec2");
			break;
		case kPropertyType_Integer :
            snprintf(sDest, maxLen, "int");
			break;
		case kPropertyType_IntegerTuple :
            snprintf(sDest, maxLen, "int2");
			break;
		case kPropertyType_Color :
            snprintf(sDest, maxLen, "rgba");
			break;            
		case kPropertyType_Quaternion:
            snprintf(sDest, maxLen, "quat");
			break;
		case kPropertyType_Vector :
            snprintf(sDest, maxLen, "vec3");
			break;
		case kPropertyType_String :
            snprintf(sDest, maxLen, "str");
			break;
		case kPropertyType_Enum :
            snprintf(sDest, maxLen, "enum");
			break;
		case kPropertyType_Bool :
            snprintf(sDest, maxLen, "bool");
			break;
		case kPropertyType_UserPtr :
			if (userTypeName != NULL) {
				snprintf(sDest, maxLen, "%s",userTypeName);
			} else {
            	snprintf(sDest, maxLen, "ptr");				
			}
			break;
		case kPropertyType_Unbound :
            snprintf(sDest, maxLen, "-int-");
            break;
        default :
            snprintf(sDest, maxLen, "unk");
            break;
            
    }
    return sDest;
}
static int hexStrToValue(const char *s, int len) {
	char tmp[256];
	if (len > 255) return 0;
	strncpy(tmp, s, len);
	return std::strtoul(tmp, NULL, 16);
}

void PropertyInstance::SetValue(const char *sValue)
{
	switch (GetPropertyType())
	{
		case kPropertyType_Float:
			property->v->float_val = (float)atof(sValue);
			break;
		case kPropertyType_FloatTuple :
			sscanf(sValue, "%f,%f",
				&property->v->float_tuple[0],
				&property->v->float_tuple[1]);
			break;
		case kPropertyType_Integer :
			property->v->int_val = atoi(sValue);
			break;
		case kPropertyType_IntegerTuple :
			sscanf(sValue, "%d,%d",
				&property->v->int_tuple[0],
				&property->v->int_tuple[1]);
			break;
		case kPropertyType_Color :
			if (sValue[0]=='#') {
				property->v->rgba[0] = hexStrToValue(&sValue[1],2);
				property->v->rgba[1] = hexStrToValue(&sValue[3],2);
				property->v->rgba[2] = hexStrToValue(&sValue[5],2);
				if (strlen(sValue) > 7) {
					property->v->rgba[3] = hexStrToValue(&sValue[7],2);
				} else
				{
					property->v->rgba[3] = 0;
				}
			} else {
				sscanf(sValue, "%f,%f,%f,%f",
					&property->v->rgba[0],
					&property->v->rgba[1],
					&property->v->rgba[2],
					&property->v->rgba[3]);
			}
			break;

		case kPropertyType_Quaternion:
			sscanf(sValue, "%f,%f,%f,%f",
				&property->v->quaternion[0],
				&property->v->quaternion[1],
				&property->v->quaternion[2],
				&property->v->quaternion[3]);
			break;
		case kPropertyType_Vector :
			sscanf(sValue, "%f,%f,%f",
				&property->v->vector[0],
				&property->v->vector[1],
				&property->v->vector[2]);
			break;
		case kPropertyType_String :
			property->v->string = strdup(sValue);	// Delete old value?
			break;
			// the integer is used as the external representation
		case kPropertyType_Enum :
			property->v->int_val = ParseEnumString(sValue,sDescription);	// contains actual enum definition
			if (property->v->int_val == -1)
			{
				// error
				property->v->int_val = 0;
			}
			break;
		case kPropertyType_Bool :
			if (!StrConfCaseCmp(sValue,"true")) {
				property->v->boolean = 1;
			} else {
				property->v->boolean = 0;
			}
			break;
		case kPropertyType_FloatArray :
			ParseFloatArrayToProperty(property, sValue);
			break;
		case kPropertyType_UserPtr :
			property->v->userdata = (void*)sValue;
			break;
		case kPropertyType_Unbound :
			unboundRawValue = strdup(sValue);
			break;
		default:
			Logger::GetLogger("PropertyInstance")->Warning("SetValue, Unsupported property type: %d",(int)GetPropertyType());
			break;
	}

	IDocNode *pNode = GetContext()->GetDocument()->FindNode(GetPluginObjectInstance());
	GetContext()->GetDocumentController()->SetDirty(pNode);
	
}

const char *PropertyInstance::GetUnboundRawValue() {
    return unboundRawValue;
}

template<typename Target, typename Source>
Target lexical_cast(Source arg, bool &ok)
{
	std::stringstream interpreter;
	Target result;

	ok = true;
	if(!(interpreter << arg) ||
	 !(interpreter >> result) ||
	 !(interpreter >> std::ws).eof()) {
		ok = false;
	}
	return result;
}

//
// Returns the property value as a string - this is the only representation there is
//
char *PropertyInstance::GetValue(char *sValueDest, int maxlen)
{
	// Initialize to empty
	snprintf(sValueDest,maxlen, "");
	switch(GetPropertyType())
	{
		case kPropertyType_Float :
			snprintf(sValueDest,maxlen,"%f",property->v->float_val);
			break;
		case kPropertyType_FloatTuple :
			snprintf(sValueDest,maxlen,"%f,%f",property->v->float_tuple[0],property->v->float_tuple[1]);
			break;
		case kPropertyType_Integer :
			snprintf(sValueDest, maxlen, "%d", property->v->int_val);
			break;
		case kPropertyType_IntegerTuple :
			snprintf(sValueDest, maxlen, "%d,%d",property->v->int_tuple[0],property->v->int_tuple[1]);
			break;
		case kPropertyType_String :
			snprintf(sValueDest, maxlen, "%s", property->v->string);
			break;
		case kPropertyType_Vector :
			snprintf(sValueDest, maxlen, "%f,%f,%f",
				property->v->vector[0],
				property->v->vector[1],
				property->v->vector[2]);
			break;
		case kPropertyType_Color :
			snprintf(sValueDest, maxlen, "%f,%f,%f,%f",
				property->v->rgba[0],
				property->v->rgba[1],
				property->v->rgba[2],
				property->v->rgba[3]);
			break;
		case kPropertyType_Quaternion:
			snprintf(sValueDest, maxlen, "%f,%f,%f,%f",
				property->v->quaternion[0],
				property->v->quaternion[1],
				property->v->quaternion[2],
				property->v->quaternion[3]);
			break;
			// Enum is integer, description holds descrivptive value
		case kPropertyType_Enum :
			if (GetEnumFromValue(sValueDest, maxlen, property->v->int_val, this->sDescription) == NULL)
			{
				// Problem
				pLogger->Error("Unable to resolve enum for current value: %d",property->v->int_val);
			}
			break;
		case kPropertyType_Bool :
			if (property->v->boolean != 0) {
				snprintf(sValueDest, maxlen, "true");
			} else {
				snprintf(sValueDest, maxlen, "false");
			}
			break;
		case kPropertyType_FloatArray :
			{
				std::string str;
				for(int i=0;i<property->v->float_array.size;i++) {
					bool ok;
					str += lexical_cast<string>(property->v->float_array.items[i], ok);
					if (i<(property->v->float_array.size-1))
						str += ",";
				}
				snprintf(sValueDest, maxlen, "%s",str.c_str());
			}
			// snprintf(sValueDest, maxlen,"NOT IMPLEMENTED: GetValue for property type FloatArray");
			// pLogger->Error("NOT IMPLEMENTED: GetValue for property type FloatArray");
//			exit(1);
			break;
		case kPropertyType_UserPtr :
			snprintf(sValueDest, maxlen, "N/A");
			break;
		default:
			pLogger->Warning("GetValue, Unsupported property type: %d",(int)GetPropertyType());
			break;
	}
	return sValueDest;
}


//////////////////////////////////////////////////////////////////////////
//
// returns the possible enumeration values
//
std::vector<std::string> PropertyInstance::GetValidEnumValues()
{
	std::vector<std::string> values;
	if (GetPropertyType() != kPropertyType_Enum) return values;
	if (!PrepareEnumString(values, this->sDescription)) {
		values.clear();
	}
	return values;
}

char *PropertyInstance::GetEnumFromValue(char *sDest, int nMax, int val, const char *def)
{
	char *sRet = NULL;
	std::vector<std::string> enumvalues;
	if (PrepareEnumString(enumvalues,def))
	{
		if ((val >= 0) && (val < (int)enumvalues.size()))
		{
			snprintf(sDest, nMax, "%s", enumvalues[val].c_str());
			sRet = sDest;
		} else
		{
			snprintf(sDest, nMax, "%s", enumvalues[0].c_str());
		}
	} // error handled by parser
	return sRet;
}

int PropertyInstance::ParseFloatArrayToProperty(Property *property, const char *val) {
	std::vector<std::string> strings;

	StrSplit(strings,val,',');

	// TODO: Need a few checks here

	property->v->float_array.size = strings.size();
	property->v->float_array.items = (float *)malloc(sizeof(float) * strings.size());
	for(int i=0;i<strings.size();i++) {
		std::string str = strings[i];
		StringUtil::Trim(str);
		float val = std::atof(str.c_str());
		property->v->float_array.items[i] = val;
	}
	return strings.size();
}

int PropertyInstance::ParseEnumString(const char *val, const char *def) {

	int iRes =-1;
	ILogger *pLogger = Logger::GetLogger("PropertyInstance");

	std::vector<std::string> enumvalues;
	if (PrepareEnumString(enumvalues,def))
	{
		size_t i;
		for(i=0;i<enumvalues.size();i++)
		{
			pLogger->Debug("%d:%s",i,enumvalues[i].c_str());
			//if (!strcmp(val, enumvalues[i].c_str()))
			if (!StrConfCaseCmp(val, enumvalues[i].c_str()))
			{
				iRes = (int)i;
				break;
			}
		}
	}
	
	return iRes;
}

//
// Verifies the syntax and returns an array with the various sub strings of the enum
// declartion
//
bool PropertyInstance::PrepareEnumString(std::vector<std::string> &strings, const char *def)
{
	bool bRes = false;
	char tmp[256];
	ILogger *pLogger = Logger::GetLogger("PropertyInstance");


	strncpy(tmp,def,256);
	char *pSplit = strchr(tmp,'=');
	if (pSplit != NULL)
	{
		// illegal
		*pSplit='\0';
		pSplit++;
		//if ((!strcmp(tmp,"enum")) && (*pSplit=='{'))
		if ((!StrConfCaseCmp(tmp,"enum")) && (*pSplit=='{'))
		{
			char *pEnd;
			pSplit++;
			// ok, enum start fine
			pEnd = strchr(pSplit,'}');
			if (pEnd != NULL)
			{
				// ok, we have a proper list of enum declarations
				// pLogger->Debug("Enum definition ok, splitting individual values");
				*pEnd = '\0';
				StrSplit(strings,pSplit,',');
				bRes = true;
			} else
			{
				// end missing
				pLogger->Error("syntax error, missing trailing '}', must be: enum={a,b,c}");
			}
		} else
		{
			// enum no ok or { missing
			pLogger->Error("syntax error, missing leading '{' or no enum, must be: enum={a,b,c}");
		}
	} else
	{
		// = missing
		pLogger->Error("syntax error, missing '=' in defintion, must be: enum={a,b,c}");
	}
	return bRes;
}

//
// Helpers - consider moving to StringUtil
//

void PropertyInstance::StrSplit(std::vector<std::string> &strings, const char *strInput, int splitChar)
{
	std::string input(strInput);
	size_t iPos = 0;
	while(iPos != -1)
	{
		size_t iStart = iPos;
		iPos = input.find(splitChar,iPos);
		if (iPos != -1)
		{
			std::string str = input.substr(iStart, iPos-iStart);
			StringUtil::Trim(str);
			strings.push_back(str);
			iPos++;
		} else
		{
			std::string str = input.substr(iStart, input.length()-iStart);
			StringUtil::Trim(str);
			strings.push_back(str);
		}
	}
}

// - this is more or less same as in Curve

// digs out a token, separated by whitespace, from the input
// returns the starting offset for next call or zero if we reached the end of the string
static int StrToken(const char *sInput, int iOffset, char *dest, int nMax)
{
	int idx = iOffset;
	int iTarget=0;
	// skip leading space
	while(isspace(sInput[idx]))
	{
		if (sInput[idx]=='\0') return 0;
		idx++;
	}
	// copy data until white space starts
	while(!isspace(sInput[idx])) 
	{
		if (sInput[idx]=='\0') return 0;
		if (iTarget > (nMax-1)) break;	// protected destination string against buffer overflow
		dest[iTarget++]=sInput[idx++];
	}
	dest[iTarget]='\0';
	return idx;
}


#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"


//#include <OpenGl/glu.h>
#include <GL/GLEW.h>


using namespace yapt;

typedef enum {		// note: make sure this matches the property enum declaration
	kParamType_Float,
	kParamType_Vec3,
	kParamType_Int,
	kParamType_Bool,
	kParamType_Texture,
} ShaderParamType;

class OpenGLShaderParameter: public PluginObjectImpl {
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);

public:
	std::string GetName();
	ShaderParamType GetType();
	std::string GetTypeName();
	bool IsType(ShaderParamType type);
	float GetFloat();
	float *GetVec3(float *out_vec3);
	int GetInt();
	bool GetBool();
	int GetTexture();

	bool WarningIssued();
	void SetWarningFlag();

protected:
	ISystem *ySys;
	Property *paramname;
	Property *typeconverter;
	Property *float_value;
	Property *int_value;	
	Property *vec3_value;
	Property *bool_value;
	Property *texture_value;

	std::string instanceName;

	bool not_found_warning_issued;
};



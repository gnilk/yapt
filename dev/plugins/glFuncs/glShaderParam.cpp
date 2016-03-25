#include "glShaderParam.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"

#include <math.h>

//#include <gl/glew.h>
#include <GL/GLEW.h>
//#include <OpenGl/glu.h>


using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLShaderParameter::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  instanceName = pInstance->GetInstanceName();
  typeconverter = pInstance->CreateProperty("type", kPropertyType_Enum, "float", "enum={float,vec3,int,bool}");  
  float_value = pInstance->CreateProperty("float", kPropertyType_Float, "0", "");
  vec3_value = pInstance->CreateProperty("vec3", kPropertyType_Vector, "0,0,0", "");
  int_value = pInstance->CreateProperty("int", kPropertyType_Integer, "0", "");
  bool_value = pInstance->CreateProperty("bool", kPropertyType_Bool, "false", "");
  this->ySys = ySys;
  not_found_warning_issued = false;
}


void OpenGLShaderParameter::Render(double t, IPluginObjectInstance *pInstance) {
}

void OpenGLShaderParameter::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
}

void OpenGLShaderParameter::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}

bool OpenGLShaderParameter::WarningIssued() {
  return not_found_warning_issued;
}
void OpenGLShaderParameter::SetWarningFlag() {
  not_found_warning_issued = true;
}


std::string OpenGLShaderParameter::GetName() {
  return instanceName;
}

ShaderParamType OpenGLShaderParameter::GetType() {
  return (ShaderParamType)typeconverter->v->int_val;   // requires enum declarations to match
}

std::string OpenGLShaderParameter::GetTypeName() {
  static std::string names[] = {"float", "vec3", "int", "bool"};
  return names[typeconverter->v->int_val];
}

bool OpenGLShaderParameter::IsType(ShaderParamType type) {
  return (typeconverter->v->int_val == (int)type);
}

float OpenGLShaderParameter::GetFloat() {
  return float_value->v->float_val;
}

float *OpenGLShaderParameter::GetVec3(float *out_vec3) {
  vDup(out_vec3, vec3_value->v->vector);
  return out_vec3;
}

int OpenGLShaderParameter::GetInt() {
  return int_value->v->int_val;
}

bool OpenGLShaderParameter::GetBool() {
  return bool_value->v->boolean;
}



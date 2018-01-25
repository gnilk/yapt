#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "Constant.h"

static yapt::ILogger *pLogger;

Constant::Constant() {

}
void Constant::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	// Input

	input[0] = pInstance->CreateProperty("float", kPropertyType_Float, "0", "");
	input[1] = pInstance->CreateProperty("vec3", kPropertyType_Vector, "0,0,0", "");
	input[2] = pInstance->CreateProperty("int", kPropertyType_Integer, "0", "");
	input[3] = pInstance->CreateProperty("bool", kPropertyType_Bool, "false", "");
	input[4] = pInstance->CreateProperty("string", kPropertyType_String, "", "");
	// Output
	output[0] = pInstance->CreateOutputProperty("float",kPropertyType_Float,"0","");
	output[1] = pInstance->CreateOutputProperty("vec3", kPropertyType_Vector, "0,0,0", "");
	output[2] = pInstance->CreateOutputProperty("int", kPropertyType_Integer, "0", "");
	output[3] = pInstance->CreateOutputProperty("bool", kPropertyType_Bool, "false", "");
	output[4] = pInstance->CreateOutputProperty("string", kPropertyType_String, "", "");

	pLogger = ySys->GetLogger("utils.Constant");
	CopyInputToOutput(); // Is this needed???
	// pLogger->Debug("Initialize, Input string is: %s\n", input[4]->v->string);
	// pLogger->Debug("Initialize, Output string is: %s\n", output[4]->v->string);
}

void Constant::CopyInputToOutput() {
	for(int i=0;i<5;i++) {
		output[i]->v = input[i]->v;
	}	
}
void Constant::Render(double t, IPluginObjectInstance *pInstance) {
}
void Constant::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	// CopyInputToOutput();
	// pLogger->Debug("PostInitialize, Input string is: %s\n", input[4]->v->string);
	// pLogger->Debug("PostInitialize, Output string is: %s\n", output[4]->v->string);
}
void Constant::PostRender(double t, IPluginObjectInstance *pInstance) {

}

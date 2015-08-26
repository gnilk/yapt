#include "glShaderBase.h"
#include "glFog.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"


#include <math.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <OpenGl/glu.h>
#endif

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


/// Draw points
void OpenGLFog::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	start = pInstance->CreateProperty("start", kPropertyType_Float, "10","");
	end = pInstance->CreateProperty("end", kPropertyType_Float, "100.0f", "");
	color = pInstance->CreateProperty("color", kPropertyType_Color, "0.0, 0.0, 0.0, 1.0", "");
	density = pInstance->CreateProperty("density", kPropertyType_Float, "0.35","");
	type = pInstance->CreateProperty("type", kPropertyType_Enum, "Linear", "enum={Linear, Exp, Exp2}");
}

void OpenGLFog::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {


}

void OpenGLFog::Render(double t, IPluginObjectInstance *pInstance) {

	static int mode[] = {GL_LINEAR, GL_EXP, GL_EXP2};
	glFogfv(GL_FOG_COLOR, color->v->rgba);
	glFogf(GL_FOG_START, start->v->float_val);
	glFogf(GL_FOG_END, end->v->float_val);
	glFogi(GL_FOG_MODE, mode[type->v->int_val]); 
	glFogf(GL_FOG_DENSITY, density->v->float_val); 
	//float fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};  
	//glFogfv(GL_FOG_COLOR, fogColor);
	//glFogf(GL_FOG_START, 10.0f);
	//glFogf(GL_FOG_END, 100.0f);
	//glFogi(GL_FOG_MODE, GL_LINEAR); 
	//glFogf(GL_FOG_DENSITY, 0.35f); 

	glEnable(GL_FOG);

}

void OpenGLFog::PostRender(double t, IPluginObjectInstance *pInstance) {
	glDisable(GL_FOG);
}

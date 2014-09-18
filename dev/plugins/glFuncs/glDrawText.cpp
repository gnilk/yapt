#include "glShaderBase.h"
#include "glDrawText.h"
#include "glFontManager.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#include <OpenGl/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

void OpenGLDrawText::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	font = pInstance->CreateProperty("font", kPropertyType_String, "Arial.ttf", "");
	fontSize = pInstance->CreateProperty("fontsize", kPropertyType_Integer, "24", "");
	text = pInstance->CreateProperty("text", kPropertyType_String, "hello world!", "");
	alignment = pInstance->CreateProperty("alignment", kPropertyType_Enum, "center","enum={none,left,right,center}");
	position = pInstance->CreateProperty("position", kPropertyType_Vector,"0,0,0","");
	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
}

void OpenGLDrawText::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	textureFont = FontManager::GetInstance(ySys)->GetFont(std::string(font->v->string), fontSize->v->int_val);
	//new Font(std::string(font->v->string), fontSize->v->int_val, face);
	textureFont->Build();

	std::string vtxshader(vshader->v->string);
	std::string fragshader(fshader->v->string);

	if (!vtxshader.empty() && !fragshader.empty()) {
	  OpenGLShaderBase::Initialize(ySys, 
	    std::string(vshader->v->string),
	    std::string(fshader->v->string));

	  OpenGLShaderBase::ReloadIfNeeded(true);
	  if (program == NULL) {
	    ySys->GetLogger("GLShaderQuad")->Error("Shader program load error");
	    exit(1);
	  }		
	  useShaders = true;
	}
}

void OpenGLDrawText::Render(double t, IPluginObjectInstance *pInstance) {
	// Render bitmaps
	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	float width = contextParams->GetFrameBufferWidth();
	float height = contextParams->GetFrameBufferHeight();

	unsigned int uniform_tex = 0;
	unsigned int uniform_color = 0;


	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}


	float sx = 2.0 / width;
	float sy = 2.0 / height;

	sx *= width/height;

//	glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
	glScalef(-1,1,1);
	textureFont->SetRenderScaling(sx,sy);

	float xp,yp;
	float str_width, str_height;
	std::string str = std::string(text->v->string);
	textureFont->Estimate(str, &str_width, &str_height);
	xp = yp = 0;
	xp = position->v->vector[0];
	yp = position->v->vector[1];
	switch(alignment->v->int_val) {
		case 0 :	// none, use position
			break;
		case 1 :
			xp = 0;
			break;
		case 2 :	// right
			xp = 1 - str_width;
			break;
		case 3 : 	// center
			xp = 0 - str_width/2.0f;
			break;
	}

//	printf("alignment: %d\n",alignment->v->int_val);


	textureFont->Draw(str,xp,yp);
	glPopMatrix();

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

}

void OpenGLDrawText::PostRender(double t, IPluginObjectInstance *pInstance) {

}

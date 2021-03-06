#include "glShaderBase.h"
#include "glDrawText.h"
#include "glFontManager.h"
#include "Tokenizer.hpp"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


#include <math.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <OpenGl/glu.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace yapt;
using namespace gnilk;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

void OpenGLDrawText::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	font = pInstance->CreateProperty("font", kPropertyType_String, "Arial.ttf", "");
	fontSize = pInstance->CreateProperty("fontsize", kPropertyType_Integer, "24", "");
	text = pInstance->CreateProperty("text", kPropertyType_String, "hello world!", "");
	alignment = pInstance->CreateProperty("alignment", kPropertyType_Enum, "center","enum={none,left,right,center}");
	position = pInstance->CreateProperty("position", kPropertyType_Vector,"0,0,0","");
	useblend = pInstance->CreateProperty("useblend", kPropertyType_Bool,"false","");
	color = pInstance->CreateProperty("color", kPropertyType_Color,"1,1,1,1","");
	alpha = pInstance->CreateProperty("alpha", kPropertyType_Float,"1","");

	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
}

void OpenGLDrawText::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	textureFont = FontManager::GetInstance(ySys)->GetFont(std::string(font->v->string), fontSize->v->int_val);
	//textureFont = FontManager::GetInstance(ySys)->LoadBitmapFont(std::string("fontmap.fnt"));
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

static bool rendered = false;

void OpenGLDrawText::Render(double t, IPluginObjectInstance *pInstance) {
	// Render bitmaps
	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	float width = contextParams->GetWindowWidth();
	float height = contextParams->GetWindowHeight();
	float px_width = contextParams->GetFrameBufferWidth();
	float px_height = contextParams->GetFrameBufferHeight();

	//printf("win (%d,%d), fb (%d:%d)\n",(int)width,(int)height,(int)px_width,(int)px_height);

	float xfac = px_width/width;
	float yfac = px_height/height;

	unsigned int uniform_tex = 0;
	unsigned int uniform_color = 0;

	if (rendered) return;
	rendered = false;

	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}
	//Window mode:
	//	win (640,360), fb (1280:720)
	//	xfac/yfac 2.000000:2.000000
	//	sx/sy 0.003125:0.005556
	//Full screen:
	//   win (1280,720), fb (2560:1600)
	//   xfac/yfac 2.000000:2.222222
	//   sx/sy 0.001563:0.003086



	float sx = 2.0 / width;
	float sy = 2.0 / height;
	//float sx = xfac / width;
	//float sy = yfac / height;

	// printf("xfac/yfac %f:%f\n",xfac,yfac);
	// printf("sx/sy %f:%f\n",sx,sy);

	sx *= width/height;
	//sy *= width/height;

	// ok for retina full screen -> but NOT in windowed mode
	// sx *= xfac;
	// sy *= yfac;


//	glActiveTexture(GL_TEXTURE0);
	glPushMatrix();
	glScalef(-1,1,1);
	textureFont->SetRenderScaling(sx,sy);

	float xp,yp;
	float str_width, str_height;
	float linepos = 0;
	std::string strBase = std::string(text->v->string);


	Tokenizer tok(strBase, "\\");
	while (true) {
		std::string str = tok.NextIncludeSpace();
		if (str.empty()) break;

		textureFont->Estimate(str, &str_width, &str_height);
		xp = yp = 0;
		xp = position->v->vector[0];
		yp = position->v->vector[1] + linepos;
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
		if (useblend->v->boolean == true) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(color->v->rgba[0], color->v->rgba[1], color->v->rgba[2], alpha->v->float_val);
	//		glColor4f(1,1,1, alpha->v->float_val);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		
		} else {
			glColor4f(color->v->rgba[0], color->v->rgba[1], color->v->rgba[2], alpha->v->float_val);
		}
		textureFont->Draw(str,xp,yp);
		linepos -= sy * 0.5 * (float)fontSize->v->int_val;
	}
	glPopMatrix();

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}
	if (useblend->v->boolean) {
		glDisable(GL_BLEND);
	}

}

void OpenGLDrawText::PostRender(double t, IPluginObjectInstance *pInstance) {

}

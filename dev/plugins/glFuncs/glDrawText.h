#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_set>


#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"
#include "glFontManager.h"

using namespace yapt;


class OpenGLDrawText: public PluginObjectImpl, public OpenGLShaderBase {
private:
	// input
	Property *font;
	Property *fontSize;
	Property *text;
	Property *vshader;
	Property *fshader;
	Property *alignment;
	Property *position;
	bool useShaders;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
	Font *textureFont;
};

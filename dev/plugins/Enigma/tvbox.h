#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"

using namespace yapt;

class EnigmaTVBox: public PluginObjectImpl {
private:
	Property *numVertex;
	Property *vertexData;
	Property *numQuads;
	Property *quadData;
	Property *wireframe;
	Property *solidcolor;

	Property *tex0;
	Property *tex1;
	Property *tex2;
	Property *tex3;
	Property *tex4;
	Property *tex5;

	// Property *mat_specular;
	// Property *mat_shininess;

	// Property *light_ambient;
	// Property *light_diffuse;
	// Property *light_specular;

public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);

private:
	void RenderTextureQuad(int quad, Property *tex);
};

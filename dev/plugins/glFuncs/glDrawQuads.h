#pragma once

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "PluginObjectImpl.h"
#include "glShaderBase.h"

using namespace yapt;

class OpenGLDrawQuads: public PluginObjectImpl, public OpenGLShaderBase {
private:
	Property *numVertex;
	Property *vertexData;
	Property *vertexUVData;
	Property *numQuads;
	Property *quadData;
	Property *vshader;
	Property *fshader;
	Property *uselighting;
	Property *wireframe;
	Property *cullface;
	Property *texture;
	Property *solidcolor;
	Property *ignoreZBuffer;

	Property *mat_specular;
	Property *mat_shininess;

	Property *light_ambient;
	Property *light_diffuse;
	Property *light_specular;

	Property *blend_func;

	bool useShaders;
public:
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);


	virtual void DrawWireFrame(int num, float *pVertex, int *pQuads);
	virtual void DrawSolidPoly(int num, float *pVertex, float *pUVData, bool bUseTexture, int *pQuads);


};

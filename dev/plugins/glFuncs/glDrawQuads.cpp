#include "glShaderBase.h"
#include "glDrawQuads.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"


#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif


/// Draw points
void OpenGLDrawQuads::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	numQuads = pInstance->CreateProperty("quadCount", kPropertyType_Integer, "0","");
	quadData = pInstance->CreateProperty("quadData", kPropertyType_UserPtr, NULL, "");

	uselighting = pInstance->CreateProperty("lighting", kPropertyType_Bool, "true", "");
	mat_specular = pInstance->CreateProperty("material_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	mat_shininess = pInstance->CreateProperty("material_shininess", kPropertyType_Float, "10.0","");

	light_ambient = pInstance->CreateProperty("light_ambient", kPropertyType_Color, "0.0, 0.0, 0.0, 1.0","");
	light_diffuse = pInstance->CreateProperty("light_diffuse", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	light_specular = pInstance->CreateProperty("light_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");

	vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	useShaders = false;
}

void OpenGLDrawQuads::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

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

void OpenGLDrawQuads::Render(double t, IPluginObjectInstance *pInstance) {
	float *pVertex = (float *) vertexData->v->userdata;
	int *pQuads = (int *) quadData->v->userdata;	

	float normal[3];
	float v1[3], v2[3];

	glColor3f(1,1,1);

	if (useShaders) {
		OpenGLShaderBase::ReloadIfNeeded();
		OpenGLShaderBase::Attach();
	}


	if (uselighting->v->boolean == true) {
		// GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		// GLfloat mat_shininess[] = { 10.0 };
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular->v->rgba);
		GLfloat _mat_shininess[] = { mat_shininess->v->float_val };
		glMaterialfv(GL_FRONT, GL_SHININESS, _mat_shininess);


		// Lights should be in their own plugin object - need to set them before transformation
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient->v->rgba);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse->v->rgba);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular->v->rgba);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glBegin(GL_QUADS);

	for(int i=0;i<numQuads->v->int_val;i++) {
		vSub(v1, &pVertex[pQuads[i*4+3]*3], &pVertex[pQuads[i*4+0]*3]);
		vSub(v2, &pVertex[pQuads[i*4+1]*3], &pVertex[pQuads[i*4+0]*3]);
		vCross(normal, v1, v2);
		vNorm(normal,normal);

	//	glColor3f(normal[0],normal[1],normal[2]);

		glNormal3fv(normal);
		glVertex3fv(&pVertex[pQuads[i*4+0]*3]);

		glNormal3fv(normal);
		glVertex3fv(&pVertex[pQuads[i*4+1]*3]);

		glNormal3fv(normal);
		glVertex3fv(&pVertex[pQuads[i*4+2]*3]);

		glNormal3fv(normal);
		glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
	}
	glEnd();
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

}

void OpenGLDrawQuads::PostRender(double t, IPluginObjectInstance *pInstance) {

}
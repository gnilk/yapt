#include "glShaderBase.h"
#include "glDrawQuads.h"

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
void OpenGLDrawQuads::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	vertexUVData = pInstance->CreateProperty("vertexUVData", kPropertyType_UserPtr, NULL, "");
	numQuads = pInstance->CreateProperty("quadCount", kPropertyType_Integer, "0","");
	quadData = pInstance->CreateProperty("quadData", kPropertyType_UserPtr, NULL, "");

	texture = pInstance->CreateProperty("texture", kPropertyType_Integer, "0", "");
	wireframe = pInstance->CreateProperty("wireframe", kPropertyType_Bool, "false", "");
	ignoreZBuffer = pInstance->CreateProperty("ignorezbuffer", kPropertyType_Bool, "false","");
	cullface = pInstance->CreateProperty("cullface", kPropertyType_Bool, "true", "");
	solidcolor = pInstance->CreateProperty("solidcolor", kPropertyType_Color, "0.75, 0.75, 0.75, 1.0", "");
	uselighting = pInstance->CreateProperty("lighting", kPropertyType_Bool, "true", "");
	mat_specular = pInstance->CreateProperty("material_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	mat_shininess = pInstance->CreateProperty("material_shininess", kPropertyType_Float, "10.0","");

	light_ambient = pInstance->CreateProperty("light_ambient", kPropertyType_Color, "0.0, 0.0, 0.0, 1.0","");
	light_diffuse = pInstance->CreateProperty("light_diffuse", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	light_specular = pInstance->CreateProperty("light_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");

	blend_func = pInstance->CreateProperty("blend_func", kPropertyType_Enum, "none", "enum={none, add}");	

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
	float *pUVData = (float *) vertexUVData->v->userdata;
	int *pQuads = (int *) quadData->v->userdata;	

	float normal[3];
	float v1[3], v2[3];

	// TEST
	// float fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};  
	// glFogfv(GL_FOG_COLOR, fogColor);
	// glFogf(GL_FOG_START, 10.0f);
	// glFogf(GL_FOG_END, 100.0f);
	// glFogi(GL_FOG_MODE, GL_LINEAR); 
	// glFogf(GL_FOG_DENSITY, 0.35f); 
	// glEnable(GL_FOG);

	// End test


	glColor3f(1,1,1);

	bool bUseTexture = false;
	if ((texture->v->int_val != 0) && (pUVData != NULL)) {
    	glBindTexture(GL_TEXTURE_2D,texture->v->int_val);
    	glEnable(GL_TEXTURE_2D);
    	bUseTexture = true;
	}
	if (ignoreZBuffer->v->boolean == true) {
		glDisable(GL_DEPTH_TEST);
	}

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

	if (cullface->v->boolean == true) {
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);
	}


	if (blend_func->v->int_val) {
		glEnable(GL_BLEND);
		switch(blend_func->v->int_val) {
			case 1 : // add
				glBlendFunc(GL_ONE, GL_ONE);
				break;
		}
	}

	if (wireframe->v->boolean == true) {
		glColor3f(solidcolor->v->rgba[0], solidcolor->v->rgba[1], solidcolor->v->rgba[2]);
		for(int i=0;i<numQuads->v->int_val;i++) {
			glBegin(GL_LINE_LOOP);
				glVertex3fv(&pVertex[pQuads[i*4+0]*3]);
				glVertex3fv(&pVertex[pQuads[i*4+1]*3]);
				glVertex3fv(&pVertex[pQuads[i*4+2]*3]);
				glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
			glEnd();
		}
	} else {
		glBegin(GL_QUADS);
		for(int i=0;i<numQuads->v->int_val;i++) {
			vSub(v1, &pVertex[pQuads[i*4+3]*3], &pVertex[pQuads[i*4+0]*3]);
			vSub(v2, &pVertex[pQuads[i*4+1]*3], &pVertex[pQuads[i*4+0]*3]);
			vCross(normal, v1, v2);
			vNorm(normal,normal);

			glNormal3fv(normal);
			if (bUseTexture) glTexCoord2fv(&pUVData[pQuads[i*4+0]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+0]*3]);

			glNormal3fv(normal);
			if (bUseTexture) glTexCoord2fv(&pUVData[pQuads[i*4+1]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+1]*3]);

			glNormal3fv(normal);
			if (bUseTexture) glTexCoord2fv(&pUVData[pQuads[i*4+2]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+2]*3]);

			glNormal3fv(normal);
			if (bUseTexture) glTexCoord2fv(&pUVData[pQuads[i*4+3]*3]);			
			glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
		}
		glEnd();
	}
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	if (cullface->v->boolean == true) {
		glDisable(GL_CULL_FACE);		
	}

	if (ignoreZBuffer->v->boolean == true) {
		glEnable(GL_DEPTH_TEST);
	}


	if (bUseTexture) {
    	glDisable(GL_TEXTURE_2D);
	}

	if (useShaders) {
		OpenGLShaderBase::Detach();
	}

	if (blend_func->v->int_val) {
		glDisable(GL_BLEND);
	}

	// TEST
	//glDisable(GL_FOG);
	// end test

}

void OpenGLDrawQuads::PostRender(double t, IPluginObjectInstance *pInstance) {

}

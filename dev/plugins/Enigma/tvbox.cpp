#include "tvbox.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"


#include <math.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/glew.h>
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
void EnigmaTVBox::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

	numVertex = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0","");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	numQuads = pInstance->CreateProperty("quadCount", kPropertyType_Integer, "0","");
	quadData = pInstance->CreateProperty("quadData", kPropertyType_UserPtr, NULL, "");

	wireframe = pInstance->CreateProperty("wireframe", kPropertyType_Bool, "false", "");
	solidcolor = pInstance->CreateProperty("solidcolor", kPropertyType_Color, "0.75, 0.75, 0.75, 1.0", "");


	tex0 = pInstance->CreateProperty("tex0", kPropertyType_Integer, "0", "");
	tex1 = pInstance->CreateProperty("tex1", kPropertyType_Integer, "0", "");
	tex2 = pInstance->CreateProperty("tex2", kPropertyType_Integer, "0", "");
	tex3 = pInstance->CreateProperty("tex3", kPropertyType_Integer, "0", "");
	tex4 = pInstance->CreateProperty("tex4", kPropertyType_Integer, "0", "");
	tex5 = pInstance->CreateProperty("tex5", kPropertyType_Integer, "0", "");


	// uselighting = pInstance->CreateProperty("lighting", kPropertyType_Bool, "true", "");
	// mat_specular = pInstance->CreateProperty("material_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	// mat_shininess = pInstance->CreateProperty("material_shininess", kPropertyType_Float, "10.0","");

	// light_ambient = pInstance->CreateProperty("light_ambient", kPropertyType_Color, "0.0, 0.0, 0.0, 1.0","");
	// light_diffuse = pInstance->CreateProperty("light_diffuse", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");
	// light_specular = pInstance->CreateProperty("light_specular", kPropertyType_Color, "1.0, 1.0, 1.0, 1.0","");

	// vshader = pInstance->CreateProperty("vertexShader", kPropertyType_String, "","");
	// fshader = pInstance->CreateProperty("fragmentShader", kPropertyType_String, "","");
	// useShaders = false;
}

void EnigmaTVBox::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void EnigmaTVBox::Render(double t, IPluginObjectInstance *pInstance) {
	float *pVertex = (float *) vertexData->v->userdata;
	int *pQuads = (int *) quadData->v->userdata;	

	float normal[3];
	float v1[3], v2[3];

	glColor3f(1,1,1);


	// if (uselighting->v->boolean == true) {
	// 	// GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// 	// GLfloat mat_shininess[] = { 10.0 };
	// 	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular->v->rgba);
	// 	GLfloat _mat_shininess[] = { mat_shininess->v->float_val };
	// 	glMaterialfv(GL_FRONT, GL_SHININESS, _mat_shininess);


	// 	// Lights should be in their own plugin object - need to set them before transformation
	// 	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient->v->rgba);
	// 	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse->v->rgba);
	// 	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular->v->rgba);

	// 	glEnable(GL_LIGHTING);
	// 	glEnable(GL_LIGHT0);
	// }

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	// DRAW TEXTURES

	// avoid Z-fighting on the lines
	glPolygonOffset(1.0, 1.0);
	glEnable(GL_POLYGON_OFFSET_FILL);

	RenderTextureQuad(0, tex0);
	RenderTextureQuad(1, tex1);
	RenderTextureQuad(2, tex2);
	RenderTextureQuad(3, tex3);
	RenderTextureQuad(4, tex4);
	RenderTextureQuad(5, tex5);

	glDisable(GL_POLYGON_OFFSET_FILL);


	glColor3f(solidcolor->v->rgba[0], solidcolor->v->rgba[1], solidcolor->v->rgba[2]);
	for(int i=0;i<numQuads->v->int_val;i++) {
		glBegin(GL_LINE_LOOP);
			glVertex3fv(&pVertex[pQuads[i*4+0]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+1]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+2]*3]);
			glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
		glEnd();		
	}

	// if (wireframe->v->boolean == true) {
	// 	glColor3f(solidcolor->v->rgba[0], solidcolor->v->rgba[1], solidcolor->v->rgba[2]);
	// 	for(int i=0;i<numQuads->v->int_val;i++) {
	// 		glBegin(GL_LINE_LOOP);
	// 			glVertex3fv(&pVertex[pQuads[i*4+0]*3]);
	// 			glVertex3fv(&pVertex[pQuads[i*4+1]*3]);
	// 			glVertex3fv(&pVertex[pQuads[i*4+2]*3]);
	// 			glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
	// 		glEnd();
	// 	}
	// } else {
	// 	glBegin(GL_QUADS);
	// 	for(int i=0;i<numQuads->v->int_val;i++) {
	// 		vSub(v1, &pVertex[pQuads[i*4+3]*3], &pVertex[pQuads[i*4+0]*3]);
	// 		vSub(v2, &pVertex[pQuads[i*4+1]*3], &pVertex[pQuads[i*4+0]*3]);
	// 		vCross(normal, v1, v2);
	// 		vNorm(normal,normal);

	// 		glNormal3fv(normal);
	// 		glVertex3fv(&pVertex[pQuads[i*4+0]*3]);

	// 		glNormal3fv(normal);
	// 		glVertex3fv(&pVertex[pQuads[i*4+1]*3]);

	// 		glNormal3fv(normal);
	// 		glVertex3fv(&pVertex[pQuads[i*4+2]*3]);

	// 		glNormal3fv(normal);
	// 		glVertex3fv(&pVertex[pQuads[i*4+3]*3]);
	// 	}
	// 	glEnd();
	// }
	// glDisable(GL_LIGHTING);
	// glDisable(GL_LIGHT0);


}
void EnigmaTVBox::RenderTextureQuad(int quad, Property *tex) {
	if (tex->v->int_val == 0) return;
	float *pVertex = (float *) vertexData->v->userdata;
	int *pQuads = (int *) quadData->v->userdata;	

	glBindTexture(GL_TEXTURE_2D,tex->v->int_val);
    glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);
		glVertex3fv(&pVertex[pQuads[quad*4+0]*3]);
		glTexCoord2f(1.0f,0.0f);
		glVertex3fv(&pVertex[pQuads[quad*4+1]*3]);
		glTexCoord2f(1.0f,1.0f);
		glVertex3fv(&pVertex[pQuads[quad*4+2]*3]);
		glTexCoord2f(0.0f,1.0f);
		glVertex3fv(&pVertex[pQuads[quad*4+3]*3]);
	glEnd();
	glDisable(GL_TEXTURE_2D);

}


void EnigmaTVBox::PostRender(double t, IPluginObjectInstance *pInstance) {

}

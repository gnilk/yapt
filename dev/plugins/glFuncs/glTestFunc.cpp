//
// test functions for open gl - i.e. old code..
//
#include "glTestFunc.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"

#include <math.h>
#include <OpenGl/glu.h>

using namespace yapt;


#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

void OpenGLRenderContext::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	this->ySys = ySys;
	clear = pInstance->CreateProperty("clear", kPropertyType_Bool, "true", "");
	fov = pInstance->CreateProperty("fov", kPropertyType_Float, "65.0", "");
}

void OpenGLRenderContext::Render(double t, IPluginObjectInstance *pInstance) {

	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	width = contextParams->GetFrameBufferWidth();
   	height = contextParams->GetFrameBufferHeight();

	glViewport(0, 0, width, height);
	// Clear color buffer to black
	if (clear->v->boolean) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	}

	// Select and setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov->v->float_val, (GLfloat) width/ (GLfloat) height, 1.0f, 1000.0f);

	// Select and setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -5.0f,    // Eye-position
			0.0f, 0.0f, 0.0f,   // View-point
			0.0f, 1.0f, 0.0f);  // Up-vector

}

void OpenGLRenderContext::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLRenderContext::PostRender(double t, IPluginObjectInstance *pInstance) {

	// TODO: Copy to texture here!

}

////////
static void renderBoundingSphere(float *mid, double radius)
{
	glColor3f(1,0.5,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1], mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,1,0.5);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double y = radius * sin((double)i * M_PI/8.0);
			double z = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0], mid[1]+y, mid[2]+z);
		}
	glEnd();

	glColor3f(0.5,0.5,1);
	glBegin(GL_LINE_LOOP);
		for(int i=0;i<16;i++)
		{
			double x = radius * sin((double)i * M_PI/8.0);
			double y = radius * cos((double)i * M_PI/8.0);
			glVertex3f(mid[0]+x, mid[1]+y, mid[2]);
		}
	glEnd();
}	

void OpenGLDrawTriangles::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	indexCount = pInstance->CreateProperty("indexCount", kPropertyType_Integer, "0","");
	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");
	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
	speed = pInstance->CreateProperty("speed", kPropertyType_Float, "100", "");
}

void OpenGLDrawTriangles::Render(double t, IPluginObjectInstance *pInstance) {
	float *vtx = (float *)vertexData->v->userdata;
	int *idx = (int *)indexData->v->userdata;
	glBegin(GL_TRIANGLES);
	int ti = 0;
	while (ti < indexCount->v->int_val) {
		glVertex3fv (&vtx[idx[ti+0]*3]);
		glVertex3fv (&vtx[idx[ti+1]*3]);
		glVertex3fv (&vtx[idx[ti+2]*3]);
		ti+=3;
	}
	glEnd();


	float mid[3] = {0,0,0};
	renderBoundingSphere(mid, 1.0f);


}

void OpenGLDrawTriangles::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLDrawTriangles::PostRender(double t, IPluginObjectInstance *pInstance) {

}

// ////
// void OpenGLDrawLines::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
// 	indexCount = pInstance->CreateProperty("indexCount", kPropertyType_Integer, "0","");
// 	indexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");
// 	vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");
// }

// void OpenGLDrawLines::Render(double t, IPluginObjectInstance *pInstance) {

// 	float *vtx = (float *)vertexData->v->userdata;
// 	int *idx = (int *)indexData->v->userdata;

// 	glColor3f(1,1,1);
// 	glBegin(GL_LINES);
// 	int li = 0;
// 	while (li < indexCount->v->int_val) {
// 		// ok, this is fake!
// 		glVertex3fv (&vtx[idx[li+0]*3]);
// 		glVertex3fv (&vtx[idx[li+1]*3]);
// 		li+=2;
// 	}
// 	glEnd();

// 	// float mid[3] = {0,0,0};
// 	// renderBoundingSphere(mid, 1.0f);
// }

// void OpenGLDrawLines::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

// }
// void OpenGLDrawLines::PostRender(double t, IPluginObjectInstance *pInstance) {

// }


static	GLuint framebuffer, texture;

void OpenGLRenderToTexture::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLRenderToTexture::Render(double t, IPluginObjectInstance *pInstance) {
	// Render to texture
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
	IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
	float width = contextParams->GetFrameBufferWidth();
   	float height = contextParams->GetFrameBufferHeight();

	glViewport(0, 0, 512, 512);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Select and setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(35.0, (GLfloat) width/ (GLfloat) height, 1.0f, 100.0f);

	// Select and setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -5.0f,    // Eye-position
			0.0f, 0.0f, 0.0f,   // View-point
			0.0f, 1.0f, 0.0f);  // Up-vector

	float mid[3];
	vIni(mid,0,0,0);
	glRotatef(t*48,1,0,0);
	glRotatef(t*64,0,1,0);
	renderBoundingSphere(mid,2);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0,0,width,height); // Ensure,if someone did change it
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glScalef(1, -1, 1);
	glTranslatef(0, -height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D,texture);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f,0.0f);
      glVertex2f(0.0f, 0.0f);
      glTexCoord2f(1.0f,0.0f);
      glVertex2f(width, 0.0f);
      glTexCoord2f(1.0f,1.0f);
      glVertex2f(width, height);
      glTexCoord2f(0.0f,1.0f);
      glVertex2f(0.0f, height);
     glEnd();  //glBegin(GL_QUADS);
    glDisable(GL_TEXTURE_2D);



}

void OpenGLRenderToTexture::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	// Setup texture here

	GLenum status;

	glGenFramebuffers(1, &framebuffer);

// Set up the FBO with one texture attachment

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		ySys->GetLogger("RenderToTexture")->Debug("OpenGL FAILED!");
		exit(1);
	}

	ySys->GetLogger("RenderToTexture")->Debug("Ok, render target configured");

}

void OpenGLRenderToTexture::PostRender(double t, IPluginObjectInstance *pInstance) {

}


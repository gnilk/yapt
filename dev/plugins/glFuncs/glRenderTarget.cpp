#include "glRenderTarget.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

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

#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

static void renderBoundingSphere(float *mid, double radius);


using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLRenderTarget::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  fov = pInstance->CreateProperty("fov", kPropertyType_Float, "65.0", "");  
  clear = pInstance->CreateProperty("clear", kPropertyType_Bool, "true", "");
  clearcol = pInstance->CreateProperty("clearcol", kPropertyType_Color, "0.0, 0.0, 0.0, 1.0","");

  textureWidth = pInstance->CreateProperty("width", kPropertyType_Integer, "512", "");  
  textureHeight = pInstance->CreateProperty("height", kPropertyType_Integer, "512", "");  

  texture = pInstance->CreateOutputProperty("texture", kPropertyType_Integer, "0", "");  
  framebuffer = pInstance->CreateOutputProperty("framebuffer", kPropertyType_Integer, "0", "");  
}

void OpenGLRenderTarget::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  GLenum status;

  // int depthParams;
  // glGetFramebufferAttachmentParameteriv(GL_RENDERBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthParams);
  // printf("DEPTH: %d\n",depthParams);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // Create frame buffer with texture
  glGenFramebuffers(1, &idFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, idFramebuffer);
  glGenTextures(1, &idTexture);
  glBindTexture(GL_TEXTURE_2D, idTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth->v->int_val, textureHeight->v->int_val, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idTexture, 0);

  // Create and attach depth buffer
  glGenRenderbuffers(1, &idDepthRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, idDepthRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth->v->int_val, textureHeight->v->int_val);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, idDepthRenderBuffer);

  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

  if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
    ySys->GetLogger("RenderToTexture")->Debug("OpenGL FAILED!");
    exit(1);
  }

  ySys->GetLogger("RenderToTexture")->Debug("Ok, render target configured (size: %dx%d)",textureWidth->v->int_val, textureHeight->v->int_val);

  this->texture->v->int_val = idTexture;
  this->framebuffer->v->int_val = idFramebuffer;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glPopAttrib();


}


void OpenGLRenderTarget::Render(double t, IPluginObjectInstance *pInstance) {
  
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glBindFramebuffer(GL_FRAMEBUFFER, idFramebuffer);

  float width = (float)textureWidth->v->int_val;
  float height = (float)textureHeight->v->int_val;

  contextParameters.SetResolution((int)width, (int)height);

  IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
  pContext->PushContextParamObject((void *)&contextParameters, "");


  glViewport(0, 0, width, height);

  // Clear color buffer to black
  if (clear->v->boolean) {
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(clearcol->v->rgba[0],clearcol->v->rgba[1],clearcol->v->rgba[2],clearcol->v->rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Select and setup the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fov->v->float_val, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

  // Select and setup the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0f, 0.0f, -8.0f,    // Eye-position
      0.0f, 0.0f, 0.0f,   // View-point
      0.0f, 0.0f, 1.0f);  // Up-vector

  float mid[3] = {0,0,0};
  glRotatef(t*48,1,0,0);
  glRotatef(t*64,0,1,0);
  renderBoundingSphere(mid, 1.5);
}


void OpenGLRenderTarget::PostRender(double t, IPluginObjectInstance *pInstance) {
  IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
  pContext->PopContextParamObject();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glPopAttrib();

}

/// --- TEST CODE
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


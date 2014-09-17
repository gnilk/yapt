#include "glFullScreenImage.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <math.h>

//#include <gl/glew.h>
#include <OpenGl/glu.h>


using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLFullScreenImage::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  texture = pInstance->CreateProperty("texture", kPropertyType_Integer, "0", "");

}

void OpenGLFullScreenImage::BeginOrtho() {
  glViewport(0,0,width,height); // Ensure,if someone did change it
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  // glScalef(1, -1, 1);
  // glTranslatef(0, -height, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void OpenGLFullScreenImage::EndOrtho(){
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void OpenGLFullScreenImage::Render(double t, IPluginObjectInstance *pInstance) {
  IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
  IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
  width = contextParams->GetFrameBufferWidth();
  height = contextParams->GetFrameBufferHeight();


  BeginOrtho();
  if (texture->v->int_val != 0) {
    glBindTexture(GL_TEXTURE_2D,texture->v->int_val);
    glEnable(GL_TEXTURE_2D);
//        glColor3f(0,0,1);

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
  } else {
    glColor3f(0,1,0);
    glBegin(GL_QUADS);
      glVertex2f(0, 0);
      glVertex2f(width, 0);
      glVertex2f(width, height);
      glVertex2f(0, height);
   glEnd();  //glBegin(GL_QUADS);
  }
  EndOrtho();
}

void OpenGLFullScreenImage::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLFullScreenImage::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}


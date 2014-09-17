#include "glShaderQuad.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include <math.h>

//#include <gl/glew.h>
#include <GL/GLEW.h>
//#include <OpenGl/glu.h>


using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLShaderQuad::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  width = ySys->GetConfigInt(kConfig_ResolutionWidth,1280);  
  height = ySys->GetConfigInt(kConfig_ResolutionWidth,720);  
  pixelshader_source = pInstance->CreateProperty("pixelshader", kPropertyType_String, "", "");
  vertexshader_source = pInstance->CreateProperty("vertexshader", kPropertyType_String, "", "");
  depthwrite = pInstance->CreateProperty("depthwrite", kPropertyType_Bool, "true", "");
  this->ySys = ySys;
}

void OpenGLShaderQuad::BeginOrtho() {

  glViewport(0,0,width,height); // Ensure,if someone did change it
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(-1, 1, -1, 1);
  //glScalef(1, -1, 1);
  //glTranslatef(0, -height, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void OpenGLShaderQuad::EndOrtho(){
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void OpenGLShaderQuad::Render(double t, IPluginObjectInstance *pInstance) {
  IContext *pContext = dynamic_cast<IBaseInstance *>(pInstance)->GetContext();
  IRenderContextParams *contextParams = (IRenderContextParams *)pContext->TopContextParamObject();
  width = contextParams->GetFrameBufferWidth();
  height = contextParams->GetFrameBufferHeight();

  BeginOrtho();

  OpenGLShaderBase::ReloadIfNeeded();


  program->Attach();
  unsigned int idResolution = program->GetUniform("iResolution");
  unsigned int idGlobalTime = program->GetUniform("iGlobalTime");
  if (idResolution != -1) {
    glUniform2f(idResolution, (float)width, (float)height);    
  }
  if (idGlobalTime != -1) {
    glUniform1f(idGlobalTime, (float)t);    
  }

  if (depthwrite->v->boolean) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
  }

  glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f,0.0f);
    glVertex2f(1, -1.0f);
    glTexCoord2f(1.0f,1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f,1.0f);
    glVertex2f(-1.0f, 1.0f);
  glEnd();  //glBegin(GL_QUADS);
  EndOrtho();
  program->Detach();
  //glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
}

void OpenGLShaderQuad::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  OpenGLShaderBase::Initialize(ySys, 
    std::string(vertexshader_source->v->string),
    std::string(pixelshader_source->v->string));

  OpenGLShaderBase::ReloadIfNeeded(true);
  if (program == NULL) {
    ySys->GetLogger("GLShaderQuad")->Error("Shader program load error");
    exit(1);
  }
}

void OpenGLShaderQuad::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}



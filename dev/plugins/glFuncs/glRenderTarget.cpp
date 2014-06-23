#include "glRenderTarget.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


//#include <gl/glew.h>
#include <GL/glfw.h>

using namespace yapt;
/////////////////////
//
// -- Render context
//
void OpenGLRenderTarget::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  
}
void OpenGLRenderTarget::Render(double t, IPluginObjectInstance *pInstance) {
  int width = 640;
  int height = 480;
  glViewport(0, 0, 640, 480);

  // Clear color buffer to black
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Select and setup the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(65.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

  // Select and setup the modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0f, 1.0f, 0.0f,    // Eye-position
      0.0f, 20.0f, 0.0f,   // View-point
      0.0f, 0.0f, 1.0f);  // Up-vector

}

void OpenGLRenderTarget::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLRenderTarget::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}

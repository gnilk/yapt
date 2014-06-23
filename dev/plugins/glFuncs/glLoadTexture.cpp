#include "glLoadTexture.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


//#include <gl/glew.h>
#include <GL/glfw.h>

using namespace yapt;

/////////////////////
//
// -- Render context
//
void OpenGLLoadTexture::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  filename = pInstance->CreateProperty("filename",kPropertyType_String,"texture.jpg","");
  outputTexture = pInstance->CreateOutputProperty("texture",kPropertyType_Integer,"0","");
}
void OpenGLLoadTexture::Render(double t, IPluginObjectInstance *pInstance) {
  GLuint textureID;

  // Generate and bind our texture ID
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Load texture from file into video memory, including mipmap levels
  if(!glfwLoadTexture2D(filename->v->string, GLFW_BUILD_MIPMAPS_BIT))
  {
    fprintf(stderr, "Failed to load texture %s\n", filename->v->string);
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Use trilinear interpolation (GL_LINEAR_MIPMAP_LINEAR)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  outputTexture->v->int_val = textureID;
}

void OpenGLLoadTexture::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void OpenGLLoadTexture::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}

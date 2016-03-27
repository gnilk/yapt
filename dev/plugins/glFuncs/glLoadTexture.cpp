#include "glLoadTexture.h"
#include "Bitmap.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"


//#include <gl/glew.h>

using namespace yapt;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <OpenGl/glu.h>
#endif

/////////////////////
//
// -- Render context
//
void OpenGLLoadTexture::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  filename = pInstance->CreateProperty("filename",kPropertyType_String,"texture.png","");
  pInstance->SetPropertyHint("filename",kPropertyHint_File);
  outputTexture = pInstance->CreateOutputProperty("texture",kPropertyType_Integer,"0","");
}
void OpenGLLoadTexture::Render(double t, IPluginObjectInstance *pInstance) {
  outputTexture->v->int_val = idTexture;
}

void OpenGLLoadTexture::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  long szData;
  void *data = ySys->LoadData(filename->v->string, 0, &szData);
  bitmap = Bitmap::LoadPNGImage(data, szData);

  if (bitmap == NULL) {
    ySys->GetLogger("gl.LoadTexture")->Error("Unable to load PNG file from: %s", filename->v->string);
    return;
  }

  glGenTextures(1, &idTexture);
  glBindTexture(GL_TEXTURE_2D, idTexture);
  //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


  // TODO:
  // - Add parametes to control clamping, magnification and mip-maps


  /* Clamping to edges is important to prevent artifacts when scaling */
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* Linear filtering usually looks best for text */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->Width(), bitmap->Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->Buffer());

  outputTexture->v->int_val = idTexture;

  ySys->GetLogger("gl.LoadTexture")->Debug("PNG File '%s' loaded ok", filename->v->string);
}

void OpenGLLoadTexture::PostRender(double t, IPluginObjectInstance *pInstance) {

  // TODO: Copy to texture here!

}

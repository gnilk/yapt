//
// Curve animation handling for Yapt, facade for goat object
//

#include <random>

#include "yapt/ySystem.h"
#include "yapt/logger.h"

#include "vec.h"

#ifdef WIN32
#include <malloc.h>
#endif

  
// Plugin objects
#include "PluginObjectImpl.h"
#include "glRenderTarget.h"
#include "glFullScreenImage.h"
#include "glLoadTexture.h"
#include "glTestFunc.h"
#include "glBasicFuncs.h"
#include "glShaderQuad.h"
#include "glShaderBase.h"
#include "glDrawPoints.h"
#include "glDrawQuads.h"
#include "glDrawLines.h"
#include "glDrawText.h"

using namespace yapt;

extern "C" {
  int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class Factory: public IPluginObjectFactory {
public:
  virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};




static Factory factory;

IPluginObject *Factory::CreateObject(ISystem *pSys, const char *identifier) {
  ILogger *pLogger = pSys->GetLogger("glFuncs.Factory");
  IPluginObject *pObject = NULL;
  pLogger->Debug("Trying '%s'", identifier);
  if (!strcmp(identifier, "gl.RenderToTexture")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLRenderTarget());
  }
  if (!strcmp(identifier, "gl.RenderContext")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLRenderContext());
  }
  if (!strcmp(identifier, "gl.Camera")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLCamera());
  }
  if (!strcmp(identifier, "gl.Transform")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLTransform());
  }
  if (!strcmp(identifier, "gl.FullScreenImage")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLFullScreenImage());
  }
  if (!strcmp(identifier, "gl.LoadTexture2D")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLLoadTexture());
  }
  if (!strcmp(identifier, "gl.DrawPoints")) {
		pObject = dynamic_cast<IPluginObject *> (new OpenGLDrawPoints());
  }
  if (!strcmp(identifier, "gl.DrawTriangles")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLDrawTriangles());
  }
  if (!strcmp(identifier, "gl.DrawLines")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLDrawLines());
  }
  if (!strcmp(identifier, "gl.DrawQuads")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLDrawQuads());
  }
  if (!strcmp(identifier, "gl.ShaderQuad")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLShaderQuad());
  }
  if (!strcmp(identifier, "gl.DrawText")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLDrawText());
  }
  if (!strcmp(identifier, "gl.RenderTest")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLRenderToTexture());
  }
  if (pObject != NULL) {
    pLogger->Debug("Ok");
  } else
    pLogger->Debug("Failed");
  return pObject;
}

static void perror() {
}

// This function must be exported from the lib/dll
int CALLCONV yaptInitializePlugin(ISystem *ySys) {

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.RenderTest");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.RenderContext");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.RenderToTexture");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Plot");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Camera");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Transform");

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawTriangles");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawPoints");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawLines");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawQuads");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawText");

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.FullScreenImage");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.ShaderQuad");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.LoadTexture2D");
  
  return 0;
}



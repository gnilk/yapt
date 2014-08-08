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
  if (!strcmp(identifier, "gl.RenderTarget")) {
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
  if (!strcmp(identifier, "gl.DrawTriangle")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLTriangle());
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

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.RenderContext");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Plot");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Camera");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Transform");

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawTriangle");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawPoints");
  // Real stuff
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.FullScreenImage");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.LoadTexture2D");
  
  return 0;
}



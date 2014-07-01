//
// Curve animation handling for Yapt, facade for goat object
//
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


using namespace yapt;

extern "C" {
  int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class Factory: public IPluginObjectFactory {
public:
  virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};



class TestTriangleGenerator: public PluginObjectImpl {
private:
  Property *numIndex;
  Property *indexData;
  Property *vertexData;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys,
      IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
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
  if (!strcmp(identifier, "gl.FullScreenImage")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLFullScreenImage());
  }
  if (!strcmp(identifier, "gl.LoadTexture2D")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLLoadTexture());
  }
  if (!strcmp(identifier, "gl.Plot")) {
//		pObject = dynamic_cast<IPluginObject *> (new YaptCurveFacade());
  }
  if (!strcmp(identifier, "gl.Rotate3f")) {
//		pObject = dynamic_cast<IPluginObject *> (new GenericCurveKey());
  }
  if (!strcmp(identifier, "gl.DrawTriangle")) {
    pObject = dynamic_cast<IPluginObject *>(new OpenGLTriangle());
  }
  if (!strcmp(identifier, "geom.Triangle")) {
    pObject = dynamic_cast<IPluginObject *>(new TestTriangleGenerator());
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
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Rotate3f");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.Camera");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.DrawTriangle");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Triangle");
  // Real stuff
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.FullScreenImage");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=gl.LoadTexture2D");
  
  return 0;
}

//
// -- Triangle
//
void TestTriangleGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  numIndex = pInstance->CreateOutputProperty("numIndex", kPropertyType_Integer, "0", "");
  indexData = pInstance->CreateOutputProperty("indexData", kPropertyType_UserPtr, NULL, "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");
}

void TestTriangleGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}

void TestTriangleGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  int numIdx = 3;
  int *pIndex = (int *) malloc(sizeof(int) * numIdx);
  float *pVertex = (float *) malloc(sizeof(float) * 3 * 3);


  vIni(&pVertex[0 * 3], 1.0f, 1.0f, 0.0f);
  vIni(&pVertex[1 * 3], -1.0f, -1.0f, 0.0f);
  vIni(&pVertex[2 * 3], 1.0f, -1.0f, 0.0f);

  pIndex[0] = 0;
  pIndex[1] = 1;
  pIndex[2] = 2;

  numIndex->v->int_val = numIdx;
  indexData->v->userdata = pIndex;
  vertexData->v->userdata = pVertex;
}

void TestTriangleGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}


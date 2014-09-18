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
#include "RayTracer.h"

using namespace yapt;

extern "C" {
  int CALLCONV yaptInitializePlugin(ISystem *ySys);
}

class Factory: public IPluginObjectFactory {
public:
  virtual IPluginObject *CreateObject(ISystem *pSys, const char *identifier);
};




class Starfield : public PluginObjectImpl {
private:
  Property *numSrcVertex;
  Property *srcVertexData;
  Property *vertexCount;
  Property *vertexData;
  // could be vector, but most likely you want just to control one component
  Property *x_movement;
  Property *y_movement;
  Property *z_movement;

  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys,
      IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

static Factory factory;

IPluginObject *Factory::CreateObject(ISystem *pSys, const char *identifier) {
  ILogger *pLogger = pSys->GetLogger("Enigma.Factory");
  IPluginObject *pObject = NULL;
  pLogger->Debug("Trying '%s'", identifier);
  // if (!strcmp(identifier, "enigma.Starfield")) {
  //   pObject = dynamic_cast<IPluginObject *>(new TestTriangleGenerator());
  // }
  if (!strcmp(identifier, "enigma.Starfield")) {
     pObject = dynamic_cast<IPluginObject *>(new Starfield());
  }
  if (!strcmp(identifier, "enigma.RayTracer")) {
     pObject = dynamic_cast<IPluginObject *>(new RayTracer());
  }
  if (!strcmp(identifier, "enigma.RayTracer.Sphere")) {
     pObject = dynamic_cast<IPluginObject *>(new Sphere());
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

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=enigma.Starfield");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=enigma.RayTracer");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=enigma.RayTracer.Sphere");
  
  return 0;
}

//
// -- Starfield, implements the movement
//
void Starfield::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
//  numVertex = pInstance->CreateProperty("numVertex", kPropertyType_Integer, "1024", "");
  numSrcVertex = pInstance->CreateProperty("numSrcVertex", kPropertyType_Integer, "0", "");
  srcVertexData = pInstance->CreateProperty("srcVertexData", kPropertyType_UserPtr, NULL, "");

  x_movement = pInstance->CreateProperty("x_movement", kPropertyType_Float, "0", "");
  y_movement = pInstance->CreateProperty("y_movement", kPropertyType_Float, "0", "");
  z_movement = pInstance->CreateProperty("z_movement", kPropertyType_Float, "0", "");

  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "1024", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");
}

void Starfield::Render(double t, IPluginObjectInstance *pInstance) {
  // implement movement here
  int nVertex = numSrcVertex->v->int_val;

  float *pVertex = (float *)vertexData->v->userdata;
  float *pSrcVertex = (float *)srcVertexData->v->userdata;


  for(int i=0;i<nVertex;i++) {
    pVertex[i*3+0] = fmod(pSrcVertex[i*3+0]+x_movement->v->float_val+2,4.0) - 2.0f;
    pVertex[i*3+1] = fmod(pSrcVertex[i*3+1]+y_movement->v->float_val+2,4.0) - 2.0f;
    pVertex[i*3+2] = fmod(pSrcVertex[i*3+2]+z_movement->v->float_val+2,4.0) - 2.0f;
  }

  vertexCount->v->int_val = nVertex;
}

void Starfield::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  int nVertex = numSrcVertex->v->int_val;

  float *pVertex = (float *)vertexData->v->userdata;
  // Reinitialized??
  if ((pVertex != NULL) && (nVertex != vertexCount->v->int_val)) {
    free(pVertex);
    pVertex = NULL;
  }
  if (pVertex == NULL) {
    pVertex = (float *)malloc(sizeof(float) * nVertex * 3);
  }

  float *pSrcVertex = (float *)srcVertexData->v->userdata;


  for(int i=0;i<nVertex;i++) {
    vDup(&pVertex[i*3], &pSrcVertex[i*3]);
  }

  vertexCount->v->int_val = nVertex;
  vertexData->v->userdata = pVertex;

}

void Starfield::PostRender(double t, IPluginObjectInstance *pInstance) {

}


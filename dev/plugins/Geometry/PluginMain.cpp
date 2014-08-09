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

class PointCloudGenerator : public PluginObjectImpl {
private:
  Property *numVertex;  // Num to be generated
  Property *vertexCount;  // Number actually generated (output)
  Property *vertexData;
  Property *range;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys,
      IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

class CubeGenerator : public PluginObjectImpl {
private:
  Property *scale;
  Property *vertexCount;
  Property *vertexData;
  Property *triangleCount;
  Property *triangleData;

public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys,
      IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);

};

static Factory factory;

IPluginObject *Factory::CreateObject(ISystem *pSys, const char *identifier) {
  ILogger *pLogger = pSys->GetLogger("Geometry.Factory");
  IPluginObject *pObject = NULL;
  pLogger->Debug("Trying '%s'", identifier);
  if (!strcmp(identifier, "geom.Triangle")) {
    pObject = dynamic_cast<IPluginObject *>(new TestTriangleGenerator());
  }
  if (!strcmp(identifier, "geom.PointCloud")) {
    pObject = dynamic_cast<IPluginObject *>(new PointCloudGenerator());
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

  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Triangle");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.PointCloud");
  
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

//
// -- Point Cloud
//
void PointCloudGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  numVertex = pInstance->CreateProperty("numVertex", kPropertyType_Integer, "1024", "");
  range = pInstance->CreateProperty("range", kPropertyType_Vector, "1.0, 1.0, 1.0", "");

  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "1024", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");
}

void PointCloudGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}

void PointCloudGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  int nVertex = numVertex->v->int_val;

  float *pVertex = (float *)vertexData->v->userdata;

  // Being re-initialized?
  if ((pVertex != NULL) && (nVertex != vertexCount->v->int_val)) {
    free (pVertex);
    pVertex = NULL;
  }

  if (pVertex == NULL) {
    pVertex = (float *)malloc(sizeof(float) * nVertex * 3);
  }


  std::default_random_engine generator;
  std::uniform_real_distribution<float> x_distribution(-range->v->vector[0],range->v->vector[0]);
  std::uniform_real_distribution<float> y_distribution(-range->v->vector[1],range->v->vector[1]);
  std::uniform_real_distribution<float> z_distribution(-range->v->vector[2],range->v->vector[2]);

  for(int i=0;i<nVertex;i++) {
    vIni(&pVertex[i*3], x_distribution(generator), y_distribution(generator), z_distribution(generator));
  }

  vertexCount->v->int_val = nVertex;
  vertexData->v->userdata = pVertex;

}

void PointCloudGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

void CubeGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  scale = pInstance->CreateProperty("range", kPropertyType_Vector, "1.0, 1.0, 1.0", "");

  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  triangleCount = pInstance->CreateOutputProperty("triangleCount", kPropertyType_Integer, "0", "");
  triangleData = pInstance->CreateOutputProperty("triangleData", kPropertyType_UserPtr, NULL, "");

}

void CubeGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}

void CubeGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  static float cubeVertex[] = {
     1,  1, -1 , 
     1, -1, -1,
    -1, -1, -1,
    -1,  1, -1,
     1,  1,  1,
     1, -1,  1,
    -1, -1,  1,
    -1,  1,  1,
  };

  static int cubeIndex[] = 
  { 0,1,2, 2,3,0, 
  7,6,5, 5,4,7, 
  0,4,5, 5,1,0, 
  3,2,6, 6,7,3, 
  7,4,0, 0,3,7, 
  1,5,6, 6,2,1
  };

  float *pVertex  = (float *)vertexData->v->userdata;
  int *pIndex = (int *)triangleData->v->userdata;

  // first time?
  if (pVertex == NULL) {
    pVertex = (float *)malloc(sizeof(float) * 3 * 8);
    pIndex = (int *)malloc(sizeof(int)*6*2*3);    
  }

  for(int i=0;i<8;i++) {
    vScale(&pVertex[i*3], &cubeVertex[i*3], scale->v->vector[0], scale->v->vector[1], scale->v->vector[2]);
  }
  memcpy(pIndex, cubeIndex, sizeof(int)*6*2*3);

  vertexCount->v->int_val = 8;
  triangleCount->v->int_val = 6*2;
  vertexData->v->userdata = pVertex;
  triangleData->v->userdata = pIndex;
}

void CubeGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}


/*
Mesh *Mesh::CreateCube()
{
  static Goat::Vector<FTYPE> cubeVertex[] = {
    Goat::Vector<FTYPE>(  1,  1, -1 ), 
    Goat::Vector<FTYPE>(  1, -1, -1 ),
    Goat::Vector<FTYPE>( -1, -1, -1 ),
    Goat::Vector<FTYPE>( -1,  1, -1 ),
    Goat::Vector<FTYPE>(  1,  1,  1 ),
    Goat::Vector<FTYPE>(  1, -1,  1 ),
    Goat::Vector<FTYPE>( -1, -1,  1 ),
    Goat::Vector<FTYPE>( -1,  1,  1 ),
  };

  static int cubeIndex[] = 
  { 0,1,2, 2,3,0, 
  7,6,5, 5,4,7, 
  0,4,5, 5,1,0, 
  3,2,6, 6,7,3, 
  7,4,0, 0,3,7, 
  1,5,6, 6,2,1
  };


  Mesh *pMesh = new Mesh();

  pMesh->SetVertexArray(cubeVertex, 8);
  pMesh->SetIndexArray(cubeIndex, 6*2*3);

  return pMesh;
}
  */


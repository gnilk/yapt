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
#include "ArrayGenerator.h"
#include "CylinderGenerator.h"
#include "PyramidGenerator.h"
#include "CubeGenerator.h"
#include "SphereGenerator.h"
#include "QuadGenerator.h"
#include "PointCloudGenerator.h"

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

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


class Edge
{

public:
  Edge(int i1, int i2) {
    if (i1>i2) {
      this->i1 = i1;
      this->i2 = i2;      
    } else {
      this->i1 = i2;
      this->i2 = i1;
    }
  }

  bool IsEqual(Edge &other) {
    if ((this->i1 == other.i1) && (this->i2 == other.i2)) 
      return true;
    return false;
  }
public:
  int i1, i2;
};


class EdgeList : public PluginObjectImpl {
private:
  Property *inVertexCount;
  Property *inVertexData;
  Property *inTriangleCount;
  Property *inIndexData;
  Property *optimizeAdjecent;

  Property *vertexCount;
  Property *vertexData;
  Property *indexCount;
  Property *indexData;

  std::vector<Edge> edges;

public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys,
      IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);

private:
  bool AddEdge(int i1, int i2);

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
  if (!strcmp(identifier, "geom.Cube")) {
    pObject = dynamic_cast<IPluginObject *>(new CubeGenerator());
  }
  if (!strcmp(identifier, "geom.Pyramid")) {
    pObject = dynamic_cast<IPluginObject *>(new PyramidGenerator());
  }
  if (!strcmp(identifier, "geom.Sphere")) {
    pObject = dynamic_cast<IPluginObject *>(new SphereGenerator());
  }
  if (!strcmp(identifier, "geom.EdgeList")) {
    pObject = dynamic_cast<IPluginObject *>(new EdgeList());
  }
  if (!strcmp(identifier, "geom.Quad")) {
    pObject = dynamic_cast<IPluginObject *>(new QuadGenerator());
  }
  if (!strcmp(identifier, "geom.Cylinder")) {
    pObject = dynamic_cast<IPluginObject *>(new CylinderGenerator());
  }
  if (!strcmp(identifier, "geom.Array")) {
    pObject = dynamic_cast<IPluginObject *>(new ArrayGenerator());
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
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Cube");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Pyramid");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.EdgeList");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Sphere");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Quad");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Cylinder");
  ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory), "name=geom.Array");
  
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
// Computes an optimized edgelist for a triangle mesh (i.e. removes duplicate edges)
// Very slow and inefficent
//
// Note: This does not remove edges 
//
void EdgeList::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  inVertexCount = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0", "");
  inVertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  inTriangleCount = pInstance->CreateProperty("triangleCount", kPropertyType_Integer, "0", "");
  inIndexData = pInstance->CreateProperty("indexData", kPropertyType_UserPtr, NULL, "");

  // perhaps add a boolean..  =)
  optimizeAdjecent = pInstance->CreateProperty("optimizeAdjecent", kPropertyType_Bool, "true", "");


  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  indexCount = pInstance->CreateOutputProperty("indexCount", kPropertyType_Integer, "0", "");
  indexData = pInstance->CreateOutputProperty("indexData", kPropertyType_UserPtr, NULL, "");

}

// TODO: Take normal into account
bool EdgeList::AddEdge(int i1, int i2) {
  Edge edge(i1, i2);
  for(int i=0;i<edges.size();i++) {
    if (edges[i].IsEqual(edge)) {
      // TODO:
      //  1) check if 'optimizeAdjecent' is true
      if (optimizeAdjecent->v->boolean) {
      //  2) check if normal of incoming face is equal
      //  3) tag edge as 'invalid'
        
      }
      return true;
    }
  }
  edges.push_back(edge);
  return false;
}

void EdgeList::Render(double t, IPluginObjectInstance *pInstance) {

  edges.clear();

  int *pInput = (int *)inIndexData->v->userdata;  
  int numTri = inTriangleCount->v->int_val;

  for(int i=0;i<numTri;i++) {
    int i1 = pInput[i*3+0];
    int i2 = pInput[i*3+1];
    int i3 = pInput[i*3+2];
    // TODO: Calculate and supply normal
    AddEdge(i1,i2);
    AddEdge(i2,i3);
    AddEdge(i3,i1);
  }

  if (optimizeAdjecent->v->boolean) {
    // TODO: purge all invalid edges    
  }

  indexCount->v->int_val = edges.size();
  int *pIndex = (int *)malloc(sizeof(int) * edges.size() * 2);
  for(int i=0;i<edges.size();i++) {
    pIndex[i*2+0] = edges[i].i1;
    pIndex[i*2+1] = edges[i].i2;
  }
  indexData->v->userdata = pIndex;

  // indexCount->v->int_val = inTriangleCount->v->int_val;
  // indexData->v->userdata = inIndexData->v->userdata;

  vertexData->v->userdata = inVertexData->v->userdata;
  vertexCount->v->userdata = inVertexCount->v->userdata;
}

void EdgeList::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {


}

void EdgeList::PostRender(double t, IPluginObjectInstance *pInstance) {

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


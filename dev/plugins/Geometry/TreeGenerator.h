#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"

struct Vertex {
  float v[3];
};
class TreeGenerator : public PluginObjectImpl {
private:
// input
  Property *rotrange;
  Property *distmin;
  Property *distmax;
  Property *branchmin;
  Property *branchmax;
  Property *seed;
  Property *maxdepth;
  Property *maxbranches;

// output
  Property *vertexCount;  // Number actually generated (output)
  Property *vertexData;
  Property *quadCount;  // Number actually generated (output)
  Property *quadData;
  Property *lineCount;  // Number actually generated (output)
  Property *lineData;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
  void Generate(float *start, float *direction, int depth);
  bool AddSegment(float *p1, float *p2);
  int AddVertex(float *p);
  float *RotateX (float *out_v, float *v, float ang);
  float *RotateY (float *out_v, float *v, float ang);
  float *RotateZ (float *out_v, float *v, float ang);

  void BuildRotation(float *out_m);
  void BuildRotationMatrix(float *out_m, float xv, float yv, float zv);

  float *RotateXm (float *out_m, float *m, float ang);
  float *RotateYm (float *out_m, float *m, float ang);
  float *RotateZm (float *out_m, float *m, float ang);


private:
  //std::vector<float[3]> vertices;
  std::vector<Vertex> vertices;
  IPluginObjectInstance *instance;
  float *pVertex;
  int nVertex;
  int *pLines;
  int nLines;
};

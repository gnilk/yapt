#pragma once

#include "yapt/ySystem.h"

  
// Plugin objects
#include "PluginObjectImpl.h"


class CityObject {
public:
  CityObject();
  void SetPos(float x, float y, float z);
  void SetHeight(float h);
  void Prepare();
  void Generate(float *vtx_dest, float *uv_dest, int *line_dest, int *quad_dest, int vIndexOffset);

  int GetLineCount();
  int GetVertexCount();
  int GetQuadCount();
private:
  float position[3];
  float height;

  int   vcount;
  int   lcount;
  int   qcount;
};

class CityGenerator : public PluginObjectImpl {
private:
  Property *step;
  Property *num_objects;
  Property *pos_range;
  Property *max_height;


  // input
  // Property *in_vertexCount;  
  // Property *in_vertexData;
  // Property *in_quadCount;  
  // Property *in_quadData;
  // Property *in_lineCount;  
  // Property *in_lineData;

// output
  Property *out_vertexCount;  // Number actually generated (output)
  Property *out_vertexData;
  Property *out_vertexUVData;
  Property *out_quadCount;  // Number actually generated (output)
  Property *out_quadData;
  Property *out_lineCount;  // Number actually generated (output)
  Property *out_lineData;

  Property *out_roadVertexCount;  // Number actually generated (output)
  Property *out_roadVertexData;
  Property *out_roadQuadCount;  // Number actually generated (output)
  Property *out_roadQuadData;
  Property *out_roadLineCount;  // Number actually generated (output)
  Property *out_roadLineData;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
private:
  void GenerateRoads();
  void GenerateBuildings();

private:  // building 
  float Bin(float v);
  float BinSpec(float v, int bs);
  bool ShouldCreate(float x, float y, float z);
  std::map<int, bool> objectmap;

private:  // roads
  int MakeTurn(float *pNext, float *pOrigin, int from, int direction, float dist);
  bool CheckRange(float *v);
  void Road(int idxCurrent, float *pOrigin, int from, int direction, float dist, int depth);
  int AddRoadSegment(int idxCurrent, float *pNext);
  int AddRoadVertex(float *pVertex);
  int AddRoadLine(int idx1, int idx2);
  int AddRoadQuad(int idxFrom, int idxTo);

  // road variables
  int *roadlines;
  int *roadquads;
  float *roadvertex;
  int numRoadLines;
  int numRoadVertex;
  int numRoadQuads;
};

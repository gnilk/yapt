#include "PyramidGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

//
// --------[ Pyramid Generator ]----------
//

void PyramidGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  scale = pInstance->CreateProperty("range", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  generatelines = pInstance->CreateProperty("generatelines", kPropertyType_Bool, "false", "");


  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  triCount = pInstance->CreateOutputProperty("triCount", kPropertyType_Integer, "0", "");
  triData = pInstance->CreateOutputProperty("triData", kPropertyType_UserPtr, NULL, "");  

  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
}

void PyramidGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}

void PyramidGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

  static float pyrVertex[] = {
     1,  1, -1, 
     1, -1, -1,
    -1, -1, -1,
    -1,  1, -1,
     0,  0,  1,
  };

  static int pyrTriIndex[] = 
  { 0,1,2, 2,3,0, 
    0,1,4,
    1,2,4,
    2,3,4,
    3,0,4
  };

  static int pyrLinesIndex[] = 
  { 0,1,
    1,2,
    2,3,
    3,0,
    0,4,
    1,4,
    2,4,
    3,4,
  };



  float *pVertex  = (float *)vertexData->v->userdata;
  int *pIndex = (int *)triData->v->userdata;
  int *pLines = (int *)lineData->v->userdata;

  // already allocated?
  if (pVertex != NULL) {
    free(pVertex);
  }
  if (pIndex != NULL) {
    free(pIndex);
  }
  if (pLines != NULL) {
    free(pLines);
  }

  pVertex = (float *)malloc(sizeof(float) * 3 * 5);
  pIndex = (int *)malloc(sizeof(int) * 6 * 3);
  pLines = (int *)malloc(sizeof(int) * 8 * 2);

  for(int i=0;i<5;i++) {
    vScale(&pVertex[i*3], &pyrVertex[i*3], scale->v->vector[0], scale->v->vector[1], scale->v->vector[2]);
  }
  memcpy(pIndex, pyrTriIndex, sizeof(int)*6*3);
  memcpy(pLines, pyrLinesIndex, sizeof(int)*8*2);


  vertexCount->v->int_val = 5;
  vertexData->v->userdata = pVertex;

  triCount->v->int_val = 6;
  triData->v->userdata = pIndex;

  lineCount->v->int_val = 8;
  lineData->v->userdata = pLines;
}

void PyramidGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

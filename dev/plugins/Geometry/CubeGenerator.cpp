#include "CubeGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

//
// --------[ Cube Generator ]----------
//

void CubeGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  scale = pInstance->CreateProperty("range", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  generatequads = pInstance->CreateProperty("generatequads", kPropertyType_Bool, "true", "");
  generatelines = pInstance->CreateProperty("generatelines", kPropertyType_Bool, "false", "");


  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
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

  static int cubeTriIndex[] = 
  { 0,1,2, 2,3,0, 
  7,6,5, 5,4,7, 
  0,4,5, 5,1,0, 
  3,2,6, 6,7,3, 
  7,4,0, 0,3,7, 
  1,5,6, 6,2,1
  };

  static int cubeQuadIndex[] = 
  { 3,2,1,0, 
    4,5,6,7, 
    1,5,4,0,
    7,6,2,3, 
    3,0,4,7, 
    2,6,5,1, 
  };

  static int cubeLinesIndex[] = 
  { 0,1,
    1,2,
    2,3,
    3,0,
    4,5,
    5,6,
    6,7,
    7,4,
    0,4,
    1,5,
    2,6,
    3,7,
  };



  float *pVertex  = (float *)vertexData->v->userdata;
  int *pIndex = (int *)quadData->v->userdata;
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

  pVertex = (float *)malloc(sizeof(float) * 3 * 8);
  pIndex = (int *)malloc(sizeof(int) * 6 * 4);
  pLines = (int *)malloc(sizeof(int) * 12 * 2);

  for(int i=0;i<8;i++) {
    vScale(&pVertex[i*3], &cubeVertex[i*3], scale->v->vector[0], scale->v->vector[1], scale->v->vector[2]);
  }
  memcpy(pIndex, cubeQuadIndex, sizeof(int)*6*4);
  memcpy(pLines, cubeLinesIndex, sizeof(int)*12*2);


  vertexCount->v->int_val = 8;
  vertexData->v->userdata = pVertex;

  quadCount->v->int_val = 6;
  quadData->v->userdata = pIndex;

  lineCount->v->int_val = 12;
  lineData->v->userdata = pLines;
}

void CubeGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

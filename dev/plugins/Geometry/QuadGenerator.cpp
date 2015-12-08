#include "QuadGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

void QuadGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  scale = pInstance->CreateProperty("scale", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  uvscale = pInstance->CreateProperty("uvscale", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  generatequads = pInstance->CreateProperty("generatequads", kPropertyType_Bool, "true", "");
  generatelines = pInstance->CreateProperty("generatelines", kPropertyType_Bool, "false", "");
  axis = pInstance->CreateProperty("axis", kPropertyType_Enum, "XY","enum={XY, XZ, YZ}");



  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");
  vertexUVData = pInstance->CreateOutputProperty("vertexUVData", kPropertyType_UserPtr, NULL, "");

  quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
}

void QuadGenerator::Render(double t, IPluginObjectInstance *pInstance) {

}

void QuadGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  static float quadVertex[] = {
     1,  1, 0, 
     1, -1, 0,
    -1, -1, 0,
    -1,  1, 0,
  };
  static float quadUV[] = {
     1,  0, 0, 
     1,  1, 0,
     0,  1, 0,
     0,  0, 0,
  };

  static int quadIndex[] = 
  { 
    //3,2,1,0, 
    0,1,2,3,
  };

  static int quadLinesIndex[] = 
  { 0,1,
    1,2,
    2,3,
    3,0,
  };



  float *pVertex  = (float *)vertexData->v->userdata;
  float *pUVData = (float *)vertexUVData->v->userdata;
  int *pIndex = (int *)quadData->v->userdata;
  int *pLines = (int *)lineData->v->userdata;

  // already allocated?
  if (pVertex != NULL) {
    free(pVertex);
  }
  if (pUVData != NULL) {
    free(pUVData);
  }
  if (pIndex != NULL) {
    free(pIndex);
  }
  if (pLines != NULL) {
    free(pLines);
  }

  pVertex = (float *)malloc(sizeof(float) * 3 * 4);
  pUVData = (float *)malloc(sizeof(float) * 3 * 4);
  pIndex = (int *)malloc(sizeof(int) * 1 * 4);
  pLines = (int *)malloc(sizeof(int) * 4 * 2);

  for(int i=0;i<4;i++) {
    vScale(&pVertex[i*3], &quadVertex[i*3], scale->v->vector[0], scale->v->vector[1], scale->v->vector[2]);
    vScale(&pUVData[i*3], &quadUV[i*3], uvscale->v->vector[0], uvscale->v->vector[1], uvscale->v->vector[2]);
//    ySys->GetLogger("QuadGenerator")->Debug("%d (%f, %f, %f)",i,pVertex[i*3+0], pVertex[i*3+1], pVertex[i*3+2]);
  }

  #define swap(_a_,_b_) { float tmp; tmp=(_a_); _a_=_b_; _b_=tmp; }

  switch(axis->v->int_val) {
    case 0 : // XY
      break;    // do nothing this is the default
    case 1 : // XZ
      // keep X swap Y/Z
      for(int i=0;i<4;i++) swap(pVertex[i*3+1], pVertex[i*3+2]);
      break;  
    case 2 : // YZ - keep Y swap XZ
      for(int i=0;i<4;i++) swap(pVertex[i*3+0], pVertex[i*3+2]);
      break;
  }

  #undef swap

//  memcpy(pUVData, quadUV, sizeof(float)*3*4);
  memcpy(pIndex, quadIndex, sizeof(int)*1*4);
  memcpy(pLines, quadLinesIndex, sizeof(int)*4*2);


  vertexCount->v->int_val = 4;
  vertexData->v->userdata = pVertex;
  vertexUVData->v->userdata = pUVData;

  quadCount->v->int_val = 1;
  quadData->v->userdata = pIndex;

  lineCount->v->int_val = 4;
  lineData->v->userdata = pLines;

}

void QuadGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

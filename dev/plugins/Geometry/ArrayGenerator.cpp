#include "ArrayGenerator.h"
#include "vec.h"
#include "yapt/ySystem.h"

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

void ArrayGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  num = pInstance->CreateProperty("count", kPropertyType_Integer, "4", "");
  step = pInstance->CreateProperty("step", kPropertyType_Vector, "0,0,0", "");


  in_vertexCount = pInstance->CreateProperty("vertexCount", kPropertyType_Integer, "0", "");
  in_vertexData = pInstance->CreateProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  in_quadCount = pInstance->CreateProperty("quadCount", kPropertyType_Integer, "0", "");
  in_quadData = pInstance->CreateProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  in_lineCount = pInstance->CreateProperty("lineCount", kPropertyType_Integer, "0", "");
  in_lineData = pInstance->CreateProperty("lineData", kPropertyType_UserPtr, NULL, "");  



  out_vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  out_vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  out_quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  out_quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  out_lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  out_lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
}

void ArrayGenerator::Render(double t, IPluginObjectInstance *pInstance) {
  float *pVertex  = (float *)out_vertexData->v->userdata;
  int *pQuads = (int *)out_quadData->v->userdata;
  int *pLines = (int *)out_lineData->v->userdata;
  if (pVertex != NULL) {
    free (pVertex);
  }
  if (pQuads != NULL) {
    free(pQuads);
  }
  if (pLines != NULL) {
    free(pLines);
  }

//  printf("RENDER!\n");

  int copies = num->v->int_val;
  int vCount = in_vertexCount->v->int_val * copies; // vertex count
  int qCount = in_quadCount->v->int_val * copies;   // quad count
  int lCount = in_lineCount->v->int_val * copies;   // line count


  if (vCount > 0) pVertex = (float *)malloc(sizeof(float) * 3 * vCount);
  if (qCount > 0) pQuads = (int *)malloc(sizeof(int) * 4 * qCount);
  if (lCount > 0) pLines = (int *)malloc(sizeof(int) * 2 * lCount);

  for (int i=0;i<copies;i++) {
  //  printf("copy %d\n",i);
    if (pVertex != NULL) {
        memcpy(&pVertex[3*i*in_vertexCount->v->int_val], in_vertexData->v->userdata, sizeof(float)*3*in_vertexCount->v->int_val);
        if (i>0) {
          // Advance current with previous + step (cascade)
          for(int v=0;v<in_vertexCount->v->int_val;v++) {
            vAdd(&pVertex[3*(v+i*in_vertexCount->v->int_val)],
                 &pVertex[3*(v+(i-1)*in_vertexCount->v->int_val)],
                 step->v->vector);
          }
      }
    }

    if (pLines != NULL)
      memcpy(&pLines[2*i*in_lineCount->v->int_val], in_lineData->v->userdata, sizeof(int)*2*in_lineCount->v->int_val);
    if (pQuads != NULL) {
      memcpy(&pQuads[4*i*in_quadCount->v->int_val], in_quadData->v->userdata, sizeof(int)*4*in_quadCount->v->int_val);

      // cascade quad defs
      if (i>0) {
        for(int q=0;q<4*in_quadCount->v->int_val;q++) {
          pQuads[q+4*i*in_quadCount->v->int_val] = pQuads[q+4*(i-1)*in_quadCount->v->int_val] + in_vertexCount->v->int_val;
        }        
      }
    }
  }
  // printf("in\n");
  // printf("vertices=%d\n",in_vertexCount->v->int_val);
  // printf("quads=%d\n",in_quadCount->v->int_val);
  // printf("out\n");
  // printf("vCount=%d\n",vCount);
  // printf("qCount=%d\n",qCount);

  // for(int i=0;i<qCount;i++) {
  //   printf("q %d = {%d, %d, %d, %d}\n",i, pQuads[i*4+0], pQuads[i*4+1], pQuads[i*4+2], pQuads[i*4+3]);
  // }

  out_vertexCount->v->int_val = vCount;
  out_vertexData->v->userdata = (void *)pVertex;

  out_quadCount->v->int_val = qCount;
  out_quadData->v->userdata = (void *)pQuads;

  out_lineCount->v->int_val = lCount;
  out_lineData->v->userdata = (void *)pLines;

//  printf("inVertex=%p, outVertex=%p\n", in_vertexData, out_vertexData);
 

}
void ArrayGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  
}

void ArrayGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

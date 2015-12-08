#include <random>
#include "PointCloudGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

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


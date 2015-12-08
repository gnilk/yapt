#include "CylinderGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif

void CylinderGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  radius = pInstance->CreateProperty("radius", kPropertyType_Float, "1", "");
  scale = pInstance->CreateProperty("scale", kPropertyType_Vector, "1.0, 1.0, 1.0", "");
  generatequads = pInstance->CreateProperty("generatequads", kPropertyType_Bool, "true", "");
  generatelines = pInstance->CreateProperty("generatelines", kPropertyType_Bool, "false", "");
  axis = pInstance->CreateProperty("axis", kPropertyType_Enum, "XY","enum={XY, XZ, YZ}");

  segments = pInstance->CreateProperty("segments", kPropertyType_Integer, "8", "");
  captop = pInstance->CreateProperty("captop", kPropertyType_Bool, "false", "");




  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");

  quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  

  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
}

void CylinderGenerator::Render(double t, IPluginObjectInstance *pInstance) {
  float *pVertex  = (float *)vertexData->v->userdata;
  int *pQuads = (int *)quadData->v->userdata;
  int *pLines = (int *)lineData->v->userdata;

  int seg = segments->v->int_val;

  // already allocated?
  if (pVertex != NULL) {
    free(pVertex);
  }
  if (pQuads != NULL) {
    free(pQuads);
  }
  if (pLines != NULL) {
    free(pLines);
  }

 

  pVertex = (float *)malloc(sizeof(float) * 3 * 2 * seg);
  pQuads = (int *)malloc(sizeof(int) * 4 * seg);
  pLines = (int *)malloc(sizeof(int) * 2 * 3 * seg);

  float r = radius->v->float_val;

  int pc=0;
  for(int i=0;i<seg;i++) {
    float x = r * cos(2.0 * (float)i * M_PI / (float)seg);
    float z = r * sin(2.0 * (float)i * M_PI / (float)seg);
    float y = 1.0;

    x *= scale->v->vector[0];
    y *= scale->v->vector[1];
    z *= scale->v->vector[2];

    vIni(&pVertex[i*3],x,y,z);
    y*=-1.0;
    vIni(&pVertex[i*3+seg*3],x,y,z);

    // Create lines
    pLines[i*2+0] = i;
    pLines[i*2+1] = (i+1) % seg;

    pLines[i*2+0+seg*2] = seg + i;
    pLines[i*2+1+seg*2] = seg + ((i+1) % seg);

    pLines[i*2+0+seg*4] = i;
    pLines[i*2+1+seg*4] = seg+i;

    // create quads
    pQuads[pc*4+3] = i;
    pQuads[pc*4+2] = (i+1) % seg;
    pQuads[pc*4+1] = seg + ((i+1) % seg);;
    pQuads[pc*4+0] = seg + i;
    pc++;

  }

  vertexCount->v->int_val = seg * 2;
  vertexData->v->userdata = (void *)pVertex;

  quadCount->v->int_val = pc;
  quadData->v->userdata = (void *)pQuads;

  lineCount->v->int_val = seg * 3;
  lineData->v->userdata = (void *)pLines;



}

void CylinderGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
}
void CylinderGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

#include "SphereGenerator.h"

#include "vec.h"
#include "yapt/ySystem.h"

#include <math.h>

#ifndef M_PI
  #define M_PI (3.1415926535897932384626433832795f)
#endif
//
// -- sphere
//

void SphereGenerator::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  radius = pInstance->CreateProperty("radius", kPropertyType_Float, "1", "");
  latseg = pInstance->CreateProperty("segment_lat", kPropertyType_Integer, "8", "");
  longseg = pInstance->CreateProperty("segment_long", kPropertyType_Integer, "8", "");
  generatequads = pInstance->CreateProperty("generatequads", kPropertyType_Bool, "false", "");
  generatelines = pInstance->CreateProperty("generatelines", kPropertyType_Bool, "false", "");
  flattop = pInstance->CreateProperty("flattop", kPropertyType_Bool, "false", "");

  vertexCount = pInstance->CreateOutputProperty("vertexCount", kPropertyType_Integer, "0", "");
  vertexData = pInstance->CreateOutputProperty("vertexData", kPropertyType_UserPtr, NULL, "");  
  quadCount = pInstance->CreateOutputProperty("quadCount", kPropertyType_Integer, "0", "");
  quadData = pInstance->CreateOutputProperty("quadData", kPropertyType_UserPtr, NULL, "");  
  lineCount = pInstance->CreateOutputProperty("lineCount", kPropertyType_Integer, "0", "");
  lineData = pInstance->CreateOutputProperty("lineData", kPropertyType_UserPtr, NULL, "");  
  this->ySys = ySys;
}

void SphereGenerator::Render(double t, IPluginObjectInstance *pInstance) {
  float *pVertex  = (float *)vertexData->v->userdata;
  int *pQuads = (int *)quadData->v->userdata;
  int *pLines = (int *)lineData->v->userdata;
  if (pVertex != NULL) {
    free (pVertex);
  }
  if (pQuads != NULL) {
    free(pQuads);
  }
  if (pLines != NULL) {
    free(pLines);
  }


  int yseg = latseg->v->int_val;
  int xseg = longseg->v->int_val;
  float r = radius->v->float_val;
  bool lines = generatelines->v->boolean;
  bool polys = generatequads->v->boolean;

  pVertex = (float *)malloc(sizeof(float) * xseg * (yseg - 1) * 3 + sizeof(float)*2*3);
  ySys->GetLogger("geom.sphere")->Debug("r=%f, xseg=%d, yseg=%d", r, xseg, yseg);
  if (polys) {
    ySys->GetLogger("geom.sphere")->Debug("Generating quads");    
    pQuads = (int *)malloc(sizeof(int) * xseg * yseg * 4);  
  }
  if (lines) {
    ySys->GetLogger("geom.sphere")->Debug("Generating lines");        
    pLines = (int *)malloc(sizeof(int) * xseg * yseg * 4);
  }


  int pc = 0; // polycount
  int count = 0;
  int lc = 0; // line count;

  for(int y=1;y<yseg;y++) {
    float yp = r * cos((float)y * M_PI / (float)yseg);
    float rs = r * sin((float)y * M_PI / (float)yseg);
    for(int x=0;x<xseg;x++) {

      float xp = rs * sin(2.0 * (float)x * M_PI / (float)xseg);
      float zp = rs * cos(2.0 * (float)x * M_PI / (float)xseg);

      vIni(&pVertex[count * 3], xp, yp, zp);

      if (polys) {
        if (y<(yseg-1)) {
          int p1 = (y-1) * xseg + x;
          int p2 = (y-1) * xseg + ((x+1) % xseg);
          pQuads[pc*4+0] = p1;
          pQuads[pc*4+1] = p2;
          pQuads[pc*4+2] = p2+xseg;
          pQuads[pc*4+3] = p1+xseg;

          pc++;
        }
      }
      // vertex counter
      count++;
    }
  }


  if (lines) {
    for (int y=0;y<(yseg-1);y++) {
      for(int x=0;x<xseg;x++) {
        pLines[lc*2+0] = y*xseg + x;
        pLines[lc*2+1] = y*xseg + (x+1) % xseg;
        lc++;

        if (y < (yseg-2)) {
          pLines[lc*2+0] = y*xseg + x;
          pLines[lc*2+1] = (y+1)*xseg + x;
          lc++;
        }

      }
    }
  }


  // cap top and bottom
  if (polys) {
    float yp;

    if (flattop->v->boolean) {
      yp = r * cos(1.0 * M_PI / (float)yseg);      
    } else {
      yp = r;
    }

    vIni(&pVertex[count * 3],0,yp,0);
    for(int x=0;x<xseg;x++) {
          int p1 = x;
          int p2 = ((x+1) % xseg);

          pQuads[pc*4+0] = p1;
          pQuads[pc*4+1] = count;
          pQuads[pc*4+2] = count;
          pQuads[pc*4+3] = p2;
          pc++;
    }    
    count++;

    if (flattop->v->boolean) {
      yp = r * cos((float)(yseg-1) * M_PI / (float)yseg);
    } else {
      yp = -1;
    }


    vIni(&pVertex[count * 3],0,yp,0);
    int y = yseg-1;
    for(int x=0;x<xseg;x++) {
          int p1 = (y-1) * xseg + x;
          int p2 = (y-1) * xseg + ((x+1) % xseg);

          pQuads[pc*4+0] = p2;
          pQuads[pc*4+1] = count;
          pQuads[pc*4+2] = count;
          pQuads[pc*4+3] = p1;
          pc++;
    }    
    count++;
  }


  ySys->GetLogger("geom.sphere")->Debug("vertex count=%d", count);
  ySys->GetLogger("geom.sphere")->Debug("quad count=%d", pc);
  ySys->GetLogger("geom.sphere")->Debug("line count=%d", lc);

  vertexCount->v->int_val = count;
  vertexData->v->userdata = (void *)pVertex;

  quadCount->v->int_val = pc;
  quadData->v->userdata = (void *)pQuads;

  lineCount->v->int_val = lc;
  lineData->v->userdata = (void *)pLines;
}

void SphereGenerator::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  
}

void SphereGenerator::PostRender(double t, IPluginObjectInstance *pInstance) {

}

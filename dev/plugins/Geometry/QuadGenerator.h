#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"


class QuadGenerator : public PluginObjectImpl {
private:
// input
  Property *scale;
  Property *uvscale;
  Property *generatequads;
  Property *generatelines;
  Property *axis;

// output
  Property *vertexCount;  // Number actually generated (output)
  Property *vertexData;
  Property *vertexUVData;
  Property *quadCount;  // Number actually generated (output)
  Property *quadData;
  Property *lineCount;  // Number actually generated (output)
  Property *lineData;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);

};

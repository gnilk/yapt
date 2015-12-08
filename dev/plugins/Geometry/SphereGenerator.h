#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"

class SphereGenerator : public PluginObjectImpl {
private:
// input
  Property *radius;
  Property *latseg;
  Property *longseg;
  Property *generatequads;
  Property *generatelines;
  Property *flattop;
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
  // internal
  ISystem *ySys;
};

#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"

class PyramidGenerator : public PluginObjectImpl {
private:
  // input
  Property *scale;
  Property *generatelines;
  // output
  Property *vertexCount;
  Property *vertexData;
  Property *triCount;
  Property *triData;
  Property *lineCount;
  Property *lineData;

public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

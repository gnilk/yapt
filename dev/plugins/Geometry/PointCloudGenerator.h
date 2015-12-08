#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"


class PointCloudGenerator : public PluginObjectImpl {
private:
  Property *numVertex;  // Num to be generated
  Property *vertexCount;  // Number actually generated (output)
  Property *vertexData;
  Property *range;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};

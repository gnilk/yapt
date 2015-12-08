#pragma once

#include "yapt/ySystem.h"
#include "PluginObjectImpl.h"

class CylinderGenerator : public PluginObjectImpl {
private:
  Property *radius;
  Property *scale;
  Property *segments;
  Property *generatequads;
  Property *generatelines;
  Property *axis;
  Property *captop;


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

};

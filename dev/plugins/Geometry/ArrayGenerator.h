#pragma once

#include "yapt/ySystem.h"

  
// Plugin objects
#include "PluginObjectImpl.h"


class ArrayGenerator : public PluginObjectImpl {
private:
  Property *step;
  Property *num;

  // input
  Property *in_vertexCount;  
  Property *in_vertexData;
  Property *in_quadCount;  
  Property *in_quadData;
  Property *in_lineCount;  
  Property *in_lineData;

// output
  Property *out_vertexCount;  // Number actually generated (output)
  Property *out_vertexData;
  Property *out_quadCount;  // Number actually generated (output)
  Property *out_quadData;
  Property *out_lineCount;  // Number actually generated (output)
  Property *out_lineData;
public:
  virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void Render(double t, IPluginObjectInstance *pInstance);
  virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
  virtual void PostRender(double t, IPluginObjectInstance *pInstance);

};

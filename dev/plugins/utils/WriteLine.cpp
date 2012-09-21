/*
 * WriteLine.cpp
 *
 *  Created on: Sept 21, 2012
 *      Author: gnilk
 */

#include <string>
#include "yapt/ySystem.h"
#include "noice/io/io.h"
#include "noice/io/ioutils.h"


#include "WriteLine.h"

using namespace yapt;
using namespace noice::io;

WriteLine::WriteLine() :
  string(NULL)
{

}

WriteLine::~WriteLine() {
}

void WriteLine::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
  string = pInstance->CreateProperty("string", kPropertyType_String, "--text--", "");
}
  

void WriteLine::Render(double t, IPluginObjectInstance *pInstance) {
  char tmp[256];
  IPropertyInstance *pProp=pInstance->GetPropertyInstance(0,false);
  pProp->GetValue(tmp,256);
  fprintf(stdout,"%s\n",tmp);
}

void WriteLine::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void WriteLine::PostRender(double t, IPluginObjectInstance *pInstance) {

}

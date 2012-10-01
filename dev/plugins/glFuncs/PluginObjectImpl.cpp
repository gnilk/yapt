//
// This is the abstract implementation of the interface - ease of inheritance
//
#include "PluginObjectImpl.h"

#include "yapt/ySystem.h"
#include "yapt/logger.h"

using namespace yapt;

PluginObjectImpl::PluginObjectImpl() {

}
void PluginObjectImpl::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::Render(double t, IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void PluginObjectImpl::PostRender(double t, IPluginObjectInstance *pInstance) {
}

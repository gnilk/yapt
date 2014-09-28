#include <string>
#include "yapt/ySystem.h"

#include "Container.h"

//
// This is just an empty container - good for holding other objects and grouping stuff...
//

void Container::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
}

void Container::Render(double t, IPluginObjectInstance *pInstance) {
}

void Container::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}
void Container::PostRender(double t, IPluginObjectInstance *pInstance) {

}
void Container::Signal(int channelId, const char *channelName, int sigval, double sigtime) {
}

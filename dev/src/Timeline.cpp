#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

//extern "C"
//{
//	int CALLCONV InitializeTimelinePlugin(ISystem *ySys);
//}
//
//class TimelineExecuteFactory :
//	public IPluginObjectFactory
//{
//public:
//	virtual IPluginObject *CreateObject(ISystem *pSys, const char *guid_identifier);
//};

//class TimelineExecuteObject :
//	public IPluginObject
//{
//protected:
//	//
//	Property *start;
//	Property *duration;
//	Property *object;
//	IBaseInstance *pRenderInstanceObject;
//public:
//	TimelineExecuteObject();
//	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
//	virtual void Render(double t, IPluginObjectInstance *pInstance);
//	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
//	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
//};


Timeline::Timeline() : BaseInstance(kInstanceType_Timeline){
  AddAttribute("name","timeline");
}
Timeline::~Timeline() {

}

ITimelineExecute *Timeline::AddExecuteObject(float start, float duration, char *objectName) {
  TimelineExecute *pExec= new TimelineExecute();
  pExec->SetParam(start, duration, objectName);
  executeObjects.push_back(pExec);
  return dynamic_cast<ITimelineExecute *>(pExec);
}

int Timeline::GetNumExecutors() {
  return executeObjects.size();

}
IBaseInstance *Timeline::GetExecutorAtIndex(int idx) {
  return dynamic_cast<IBaseInstance*>(executeObjects.at(idx));
}

TimelineExecute::TimelineExecute() : BaseInstance(kInstanceType_TimelineExecute) {

}

TimelineExecute::~TimelineExecute() {
}

float TimelineExecute::GetStart() {
  return start;
}

float TimelineExecute::GetDuration() {
  return duration;
}

char *TimelineExecute::GetObjectName() {
  return objectName;
}

bool TimelineExecute::ShouldRender(double t) {
  return true;
}

void TimelineExecute::SetParam(float _start, float _duration, char *_objectName) {
  start = _start;
  duration = _duration;
  objectName = strdup(_objectName);
}



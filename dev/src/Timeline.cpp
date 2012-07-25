#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

extern "C"
{
	int CALLCONV InitializeTimelinePlugin(ISystem *ySys);
}

class TimelineExecuteFactory :
	public IPluginObjectFactory
{
public:
	virtual IPluginObject *CreateObject(ISystem *pSys, const char *guid_identifier);
};

class TimelineExecuteObject :
	public IPluginObject
{
protected:
	//
	Property *start;
	Property *duration;
	Property *object;
	IBaseInstance *pRenderInstanceObject;
public:
	TimelineExecuteObject();
	virtual void Initialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void Render(double t, IPluginObjectInstance *pInstance);
	virtual void PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance);
	virtual void PostRender(double t, IPluginObjectInstance *pInstance);
};


Timeline::Timeline() : BaseInstance(kInstanceType_Timeline){

}
Timeline::~Timeline() {

}

int Timeline::GetNumExecutors() {
	return 0;
}
IBaseInstance *Timeline::GetExecutorAtIndex(int idx) {
	return NULL;
}

TimelineExecuteObject::TimelineExecuteObject()
{
}
void TimelineExecuteObject::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// video
	start = pInstance->CreateProperty("start", kPropertyType_Float, "0.0", "Start time in seconds");
	duration = pInstance->CreateProperty("duration", kPropertyType_Float, "0.0", "Duration in seconds");
	object = pInstance->CreateProperty("object", kPropertyType_String, "", "Name of object to render");
}
void TimelineExecuteObject::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance)
{
	// Need a function to find first fit for a name
	pRenderInstanceObject = ySys->GetActiveDocument()->GetObjectFromSimpleName(object->v->string);
}

void TimelineExecuteObject::Render(double t, IPluginObjectInstance *pInstance)
{
	float _start = start->v->float_val;
	float _duration = duration->v->float_val;
	if (t >= _start) {
		if ((_duration == 0.0) || ((_start+_duration) < t)) {
			// Schedule for rendering
			ITimeline *pTimeline = dynamic_cast<ITimeline *>(pInstance->GetDocumentNode()->GetParent()->GetNodeObject());
			//pTimeline->Schedule(pRenderInstanceObject);
		}
	}
}
void TimelineExecuteObject::PostRender(double t, IPluginObjectInstance *pInstance)
{
}


static TimelineExecuteFactory factory;
IPluginObject *TimelineExecuteFactory::CreateObject(ISystem *pSys, const char *identifier) {
	ILogger *pLogger = pSys->GetLogger("TimelineExecuteFactory");
	pLogger->Debug("CreateObject called for: %s",identifier);
	return dynamic_cast<IPluginObject *>(new TimelineExecuteObject());
}

// static
int CALLCONV InitializeTimelinePlugin(ISystem *ySys) {
	ySys->RegisterObject(dynamic_cast<IPluginObjectFactory *>(&factory),"name=Timeline.Execute");
	return 0;
}


/*-------------------------------------------------------------------------
File    : $Archive: Timeline.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2012-09-20, 15:50
Descr   : Implementation of the Timeline container and the execute object

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 20.09.12, FKling, Implementation
---------------------------------------------------------------------------*/
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

Timeline::Timeline() : BaseInstance(kInstanceType_Timeline){
  AddAttribute("name","timeline");
}
Timeline::~Timeline() {
    for(int i=0;i<executeObjects.size();i++) {
      delete executeObjects[i];
    }

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
  node = NULL;
  objectName = NULL;
}

TimelineExecute::~TimelineExecute() {
  if(objectName != NULL) {
    free(objectName);
  }
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
  if ((t>=start) && (t<(start+duration))) {
    return true;
  } 
  return false;
}

void TimelineExecute::SetParam(float _start, float _duration, char *_objectName) {
  start = _start;
  duration = _duration;
  if (objectName != NULL) {
    free(objectName);
  }
  objectName = strdup(_objectName);

  node = NULL;  // invalidate the cache variable
}



/*-------------------------------------------------------------------------
File    : $Archive: RenderVars.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : Defines the timers and variables available during rendering

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 04.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <assert.h>
#include <string>
#include <stack>
#include <map>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;


RenderVars::RenderVars()
{
	tGlobal = 0.0;
}

RenderVars::~RenderVars()
{
}

void RenderVars::SetTime(double tGlobal)
{
	this->tGlobal = tGlobal;
}

void RenderVars::PushLocal(double tStart)
{
	localtime.push(tStart);
}

void RenderVars::PopLocal()
{
	localtime.pop();
}

double RenderVars::GetLocalTime()
{
	double tRes = 0.0;
	if (!localtime.empty())
	{
		// stack contains start values in global time space
		tRes = tGlobal - localtime.top();
	} else
	{
		tRes = tGlobal;
	}
	return tRes;
}

double RenderVars::GetTime()
{
	return tGlobal;
}

double RenderVars::GetTimer(unsigned int idTimer)
{
	return 0.0;
}

void RenderVars::CreateTimer(unsigned int idTimer, unsigned int flags)
{
	if (timers.find(idTimer) == timers.end())
	{
		Timer *pTimer = new Timer();
		timers.insert(TimerIDPair(idTimer, pTimer));
	}
}

void RenderVars::UpdateTimer(unsigned int idTimer, double newTime)
{
	if (timers.find(idTimer) != timers.end())
	{
		Timer *pTimer = timers[idTimer];
		pTimer->Update(newTime);
	}
}

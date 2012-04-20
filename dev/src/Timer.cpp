/*-------------------------------------------------------------------------
File    : $Archive: Timer.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : Timers are simple structures enabling any plugin to create their own

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Replace bUpdate with a proper state
</pre>


\History
- 04.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include <assert.h>
#include <string>
#include <stack>

#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace yapt;

Timer::Timer()
{
	t = 0.0;
	bUpdate = true;
}
Timer::~Timer()
{
}
void Timer::Update(double newTime)
{
	if (bUpdate)
	{
		t = newTime;
	}
}

double Timer::GetTime()
{
	return t;
}
void Timer::Start()
{
	bUpdate = true;
}
void Timer::Stop()
{
	bUpdate = false;
}

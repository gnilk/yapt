/*-------------------------------------------------------------------------
File    : $Archive: FPSController.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-09-17, 15:50
Descr   : Contains logic to calculate FPS values

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 28.06.2014, FKling, Implementation

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

FPSController::FPSController() {
	Reset();
}

FPSController::~FPSController() {

}

void FPSController::Reset() {
	for(int i=0;i<FPS_CONTROLLER_TIME_WINDOW_SIZE;i++) {
		fpsWindow[i] = 0;
	}
	deltaCount = 0;
	deltaIndex = 0;
	lastUpdate = -1;	
}

void FPSController::Update(double time) {
	// we had at least one update
	if (lastUpdate > 0) {

		if (lastUpdate > time) {
			Reset();
			return;
		}

		double delta = time - lastUpdate;
		fpsCurrent = 1.0 / delta;

		// Smooth out the FPS values
		fpsWindow[deltaIndex] = 1.0 / delta;
		deltaIndex = (deltaIndex + 1) % FPS_CONTROLLER_TIME_WINDOW_SIZE;
		deltaCount++;
		if (deltaCount > FPS_CONTROLLER_TIME_WINDOW_SIZE) {
			double fpsTotal = 0;
			for(int i=0;i<FPS_CONTROLLER_TIME_WINDOW_SIZE;i++) {
				fpsTotal += fpsWindow[i];
			}
			fpsAverage = fpsTotal / (double)(FPS_CONTROLLER_TIME_WINDOW_SIZE);
		}

	}
	lastUpdate = time;
}

double FPSController::GetFPS() {
	return fpsCurrent;
}

double FPSController::GetAverageFPS() {
	return fpsAverage;
}
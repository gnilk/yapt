#pragma once
/*-------------------------------------------------------------------------
 File    : $Archive: $
 Author  : $Author: $
 Version : $Revision: $
 Orginal : 2006-07-26, 15:50
 Descr   : Event/Conditional wrapper for posix and win32
 
 
 Modified: $Date: $ by $Author: $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
   - Add Wait with timeout
 </pre>
 
 
 \History
 - 16.04.09, FKling, Implementation
 
 ---------------------------------------------------------------------------*/

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <stdbool.h>
#endif

namespace yapt
{
	class Event
	{
	private:
	#ifdef WIN32
		HANDLE hEvent;
	#else
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		bool triggered;
	#endif
	public:
		Event();
		virtual ~Event();
		
		void Trigger();
		bool IsTriggered();
		void Reset(); 
		void Wait();
		
	};
}
	

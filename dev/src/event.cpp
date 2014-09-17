/*-------------------------------------------------------------------------
 File    : $Archive: $
 Author  : $Author: $
 Version : $Revision: $
 Orginal : 2006-07-26, 15:50
 Descr   : Simple event handler, took the base off the internet, classified it and
           added the "IsTriggered" function
 
	       Was looking for a pthread Win32 Event look alike mechanism
  
 Modified: $Date: $ by $Author: $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
   ! Move this class to a "platform" / Core library which can be shared by others
 </pre>
 
 
 \History
 - 16.04.09, FKling, Imported to GOAT from other project
 - 26.04.09, Krikkit, Added Win32 implementation.
 
 ---------------------------------------------------------------------------*/

#include "yapt/event.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

using namespace yapt;

Event::Event() 
{
#ifdef WIN32
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
    triggered = false;
#endif
}

Event::~Event()
{
#ifdef WIN32
    if (hEvent != NULL)
        CloseHandle(hEvent);
#else
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
#endif
}

bool Event::IsTriggered()
{
	bool bRes = false;
#ifdef WIN32
    bRes = (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);
#else
	pthread_mutex_lock(&mutex);
	bRes = triggered;
	pthread_mutex_unlock(&mutex);
#endif
	return bRes;
}

void Event::Trigger() 
{
#ifdef WIN32
    SetEvent(hEvent);
#else
    pthread_mutex_lock(&mutex);
    triggered = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
#endif
}

void Event::Reset() 
{
#ifdef WIN32
    ResetEvent(hEvent);
#else
    pthread_mutex_lock(&mutex);
    triggered = false;
    pthread_mutex_unlock(&mutex);
#endif
}

void Event::Wait() 
{
#ifdef WIN32
    WaitForSingleObject(hEvent, INFINITE);
#else
	pthread_mutex_lock(&mutex);
	while (!triggered)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
#endif
}



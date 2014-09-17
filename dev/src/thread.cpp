/*-------------------------------------------------------------------------
 File    : $Archive: $
 Author  : $Author: $
 Version : $Revision: $
 Orginal : 2006-07-26, 15:50
 Descr   : Shit boring thread wrapper for Win32 and Posix
 
 Note: There is not explicit way to stop a thread from running except for abort it..
 
 Modified: $Date: $ by $Author: $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
   - Add ability to track threads and request your own thread!
   ! Verify under windogs (currently compiles)
   - Stop under windows is fishy, why did I do it with suspend???
   - Remove ability to suspend a running thread, just causing problems with synchronization
   - Handle multiple suspend/resume call's
   ! Proper suspend creation for POSIX [however, we can not re-suspend a thread - which you should never do anyway]
   ! Implement posix handling
   - Add functionality for synchronizing a set of threads on a checkpoint.
 </pre>
 
 
 \History
 - 05.04.09, Krikkit, Fixed type errors (Visual Studio is picky).
 - 30.04.09, FKling, FIX: Moved to Goat namespace
                     FIX: Bug in constructor caused seg-fault
                     ADD: Sleep function
				     CHG: Enum's instead of #defines
 - 16.04.09, FKling, Wrote POSIX implementation
 - 07.04.09, Krikkit, Blind fix of yielding on Windows.

 ---------------------------------------------------------------------------*/

#ifdef WIN32
#define _WIN32_WINNT    0x0400
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef Yield
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include "yapt/thread.h"
#include "yapt/event.h"

using namespace yapt;


#ifdef WIN32
void Thread::InternalExecute()
{
	iState = Thread::kThreadStateRunning;// THREAD_STATE_RUNNING;
	iExit = pFunc(pArg);
	iState = Thread::kThreadStateStopped; //THREAD_STATE_STOPPED;

}
static DWORD WINAPI funcThread_callback(LPVOID lpParam)
{
	Thread *pThread = (Thread *)lpParam;
	if (pThread != NULL)
	{
		pThread->InternalExecute();
	}
	return 0;
}
Thread::Thread(THREADFUNC pFunc, void *pArg, kThreadCreateFlags flags)
{
	int err = 0;
	int win32Flags =0;
	
	iState = kThreadStateUnknown;
	this->pFunc = pFunc;
	this->pArg = pArg;
	//	if (flags & THREAD_CREATE_SUSPENDED) win32Flags |= CREATE_SUSPENDED;
	if (flags & Thread::kThreadCreateSuspended) win32Flags |= CREATE_SUSPENDED;
	hThread = CreateThread(NULL,0,funcThread_callback,this,win32Flags,&dwThreadID);
	if (hThread != NULL)
	{
		if (flags & Thread::kThreadCreateSuspended)
		{
			iState = Thread::kThreadStateSuspended;//  THREAD_STATE_SUSPENDED;
		}
	} else
	{
		err = THREAD_ERR_CREATIONFAILED;
	}
	// Last error ??
	// return err;
}

Thread::~Thread()
{
	if (iState != Thread::kThreadStateStopped) // THREAD_STATE_STOPPED)
	{
		Stop();
	}
	CloseHandle(hThread);			
}

// static
Thread *Thread::Create(THREADFUNC pFunc, void *pArg, kThreadCreateFlags flags)
{
	return new Thread(pFunc, pArg, flags);
}
// static
void Thread::Sleep(unsigned int tMsec)
{
	::Sleep(tMsec);
}

int Thread::Start()
{
	int err = THREAD_ERR_NOERROR;
	if (iState == Thread::kThreadStateSuspended) // THREAD_STATE_SUSPENDED)
	{
		int suspcount = ResumeThread(hThread);
		
		// TODO, handle multiple resume/suspend
		if (suspcount == 1)
		{
			iState = Thread::kThreadStateRunning; //THREAD_STATE_RUNNING;
		}
		err = THREAD_ERR_NOERROR;
	} else
	{
		err = THREAD_ERR_WRONGSTATE;
	}
	return err;
}

// TODO: Verify this, stop -> suspended???
int Thread::Stop()
{
	int err = THREAD_ERR_INVALIDTHREAD;
	if (iState == Thread::kThreadStateRunning) // THREAD_STATE_RUNNING)
	{
		if (SuspendThread(hThread) > 0)
		{
			iState = Thread::kThreadStateSuspended; //THREAD_STATE_SUSPENDED;
		}
		err = THREAD_ERR_NOERROR;
	} else
	{
		err = THREAD_ERR_WRONGSTATE;
	}
	return err;
}
int Thread::Abort()
{
	int err = THREAD_ERR_INVALIDTHREAD;
	if (iState == Thread::kThreadStateRunning) // THREAD_STATE_RUNNING)
	{
		TerminateThread(hThread, 0);
		err = THREAD_ERR_NOERROR;
	} else
	{
		err = THREAD_ERR_WRONGSTATE;
	}
	return err;
}
int Thread::Join()
{
	int err = THREAD_ERR_INVALIDTHREAD;
	WaitForSingleObject(hThread, INFINITE);
	err = THREAD_ERR_NOERROR;
	return err;
}

int Thread::Yield()
{
    // Returns FALSE if it does *not* switch to another thread in which case
    // we force a delay and let the Idle Process do the busy wait or any other
    // smart thing the scheduler can do when there are no threads ready to execute.

	if (!SwitchToThread())
	    Sleep(1);
	return THREAD_ERR_NOERROR;
}

#else
//
// -------------------- POSIX Implementation
//

void Thread::InternalExecute()
{
	if (iState == Thread::kThreadStateSuspended) // THREAD_STATE_SUSPENDED)
	{
		pSuspendEvent->Wait();
	}
	// Stopped during suspension???
	if (iState != Thread::kThreadStateStopped) // THREAD_STATE_STOPPED)
	{
		iState = Thread::kThreadStateRunning;
		iExit = pFunc(pArg);
		iState = Thread::kThreadStateStopped;	
	}
	pthread_exit(NULL);
}

static void *funcThread_callback(void *lpParam)
{
	Thread *pThread = (Thread *)lpParam;
	if (pThread != NULL)
	{
		pThread->InternalExecute();
	}
	return NULL;
}

Thread::Thread(THREADFUNC pFunc, void *pArg, kThreadCreateFlags flags)
{
	int err = THREAD_ERR_NOERROR;
	
	iState = Thread::kThreadStateUnknown;
	this->pFunc = pFunc;
	this->pArg = pArg;
	
	pthread_attr_init(&attr);	
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	// Always..
	
	if (flags & Thread::kThreadCreateSuspended)
	{
		// Suspended via pthread_create_suspended_np is not portable (np)
		// - see: http://lists.apple.com/archives/unix-porting/2002/Oct/msg00106.html
		pSuspendEvent = new Event();
		pSuspendEvent->Reset();
		iState = Thread::kThreadStateSuspended; // THREAD_STATE_SUSPENDED;
	
	} else
	{
		pSuspendEvent = NULL;
	}
	
	if (!pthread_create(&hThread, &attr, funcThread_callback, this)) err = THREAD_ERR_CREATIONFAILED;
	
	// what do we do with err???
	// lasterr = err;     // ??
}

// static
void Thread::Sleep(unsigned int tMsec)
{
	usleep(1000*tMsec);
}

// static
Thread *Thread::Create(THREADFUNC pFunc, void *pArg, kThreadCreateFlags flags)
{
	return new Thread(pFunc, pArg, flags);
}

Thread::~Thread()
{
	if (iState != Thread::kThreadStateStopped)
	{
		Stop();
	}
	pthread_attr_destroy(&attr);
}

int Thread::Start()
{
	int err = THREAD_ERR_NOERROR;
	if ((iState == Thread::kThreadStateSuspended) && (pSuspendEvent != NULL))
	{
		pSuspendEvent->Trigger();
	} else
	{
		err = THREAD_ERR_WRONGSTATE;
	}
	return err;
}

int Thread::Stop()
{
	int err = THREAD_ERR_NOTSUPPORTED;
	return err;
}

int Thread::Abort()
{
	int err = THREAD_ERR_NOTSUPPORTED;
	return err;
}

int Thread::Join()
{
	void *ret;
	int err = THREAD_ERR_NOERROR;
	if ((iState == Thread::kThreadStateSuspended) && (pSuspendEvent != NULL))
	{
		iState = Thread::kThreadStateStopped; //THREAD_STATE_STOPPED;
		pSuspendEvent->Trigger();
		pthread_join(hThread, &ret);
	} else 
	if(iState == Thread::kThreadStateRunning)
	{
		pthread_join(hThread, &ret);
		iState = Thread::kThreadStateStopped; //THREAD_STATE_STOPPED;
	} else
	{
		err = THREAD_ERR_WRONGSTATE;
	}
		
	return err;
}

int Thread::Yield()
{
	pthread_yield_np();
	return THREAD_ERR_NOERROR;
}
#endif

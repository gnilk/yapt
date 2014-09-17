#pragma once
/*-------------------------------------------------------------------------
 File    : $Archive: $
 Author  : $Author: $
 Version : $Revision: $
 Orginal : 2006-07-26, 15:50
 Descr   : Mutex handling for Posix and Win32
 
 
 Modified: $Date: $ by $Author: $
 ---------------------------------------------------------------------------
 TODO: [ -:Not done, +:In progress, !:Completed]
 <pre>
 - Review "delete this" in implementation
 </pre>
 
 
 \History
 - 30.04.09, FKling, POSIX implementation
 - 28.04.09, FKling, Win32 Implementation
 
 ---------------------------------------------------------------------------*/

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef Yield
#else
#include <pthread.h>
#endif


namespace yapt
{
	// Mutex interface
	struct IMutex
	{
	public:
		virtual void Initialize()=0;
		virtual void Dispose()=0;
		virtual void Enter()=0;
		virtual void Leave()=0;
	};
#ifdef WIN32
	// Win32 implementation
	class Mutex_Win32 : public IMutex
	{
	protected:
		CRITICAL_SECTION cs;
	public:
		__inline void Initialize()
		{
			InitializeCriticalSection(&cs);
		}
		__inline void Dispose()
		{
			DeleteCriticalSection(&cs);
			delete this;		// keep this?
		}
		__inline void Enter()
		{
			EnterCriticalSection(&cs);
		}
		__inline void Leave()
		{
			LeaveCriticalSection(&cs);
		}
	};
#else
	// Win32 implementation
	class Mutex_Posix : public IMutex
		{
		protected:
			pthread_mutex_t mtx;
		public:
			__inline void Initialize()
			{
				pthread_mutex_init(&mtx,NULL);
			}
			__inline void Dispose()
			{
				pthread_mutex_destroy(&mtx);
				delete this;		// Keep this??
			}
			__inline void Enter()
			{
				pthread_mutex_lock(&mtx);
			}
			__inline void Leave()
			{
				pthread_mutex_unlock(&mtx);
			}
		};
#endif
	// Platform neutral front end
	class Mutex : public IMutex
	{
	protected:
		IMutex *pImpl;	// Interface for platform implementation
	public:
		Mutex()
		{
			// Create platform impleentation 
			// here
#ifdef WIN32
			pImpl = new Mutex_Win32();
#else
			pImpl = new Mutex_Posix();
#endif
		}
		__inline virtual void Initialize()
		{
			pImpl->Initialize();
		}
		__inline virtual void Dispose()
		{
			pImpl->Dispose();
		}
		__inline virtual void Enter()
		{
			pImpl->Enter();
		}
		__inline virtual void Leave()
		{
			pImpl->Leave();
		}
		
	};
	
}


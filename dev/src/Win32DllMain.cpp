#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL,     // DLL module handle
		    DWORD fdwReason,        // reason called
		    LPVOID lpvReserved)     // reserved
{
	switch (fdwReason)
	{ 
		// The DLL is loading due to process 
		// initialization or a call to LoadLibrary. 
	case DLL_PROCESS_ATTACH: 

		break;

		// The attached process creates a new thread. 
	case DLL_THREAD_ATTACH: 

		break; 

		// The thread of the attached process terminates.
	case DLL_THREAD_DETACH: 

		break; 

		// The DLL unloading due to process termination or call to FreeLibrary. 
	case DLL_PROCESS_DETACH: 

		break; 

	default: 
		break; 
	}

	return TRUE; 
	UNREFERENCED_PARAMETER(hinstDLL); 
	UNREFERENCED_PARAMETER(lpvReserved); 
}

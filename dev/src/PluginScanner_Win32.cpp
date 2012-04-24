/*-------------------------------------------------------------------------
File    : $Archive: PluginScanner_Win32.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements a Plugin (.dll) scanner for windows, scans all files
          recurisvely (option) and tries to identify those with a properly
	  exposed function.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 17.10.09, FKling, Implementation

---------------------------------------------------------------------------*/
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace yapt;


// add this to the list of extensions we support
static const char *lExtensions[]=
{
	".dll",
	NULL
};

#define kDLL_Export ("yaptInitializePlugin")

// returns the extension of a file name
std::string PluginScanner_Win32::GetExtension(std::string &pathName)
{
	std::string ext;
	// don't use size_t for l since we have a l>0 check which would never kick in if unsigned
	int l = (int)(pathName.length())-1;
	while ((l>0) && (pathName[l]!='.'))
	{
    ext.insert(ext.begin(),pathName[l]);
		l--;
	}
	if ((l > 0) && (pathName[l]=='.'))
	{
		ext.insert(0,".");
	}
	return ext;
}

// Check extension from the list of supported extensions
bool PluginScanner_Win32::IsExtensionOk(std::string &extension)
{
	int i;
	for (i=0;lExtensions[i]!=NULL;i++)
	{
		if (!strcmp(extension.c_str(),lExtensions[i]))
		{
			return true;
		}
	}
	return false;
}

//
void PluginScanner_Win32::TryLoadLibrary(std::string &pathName)
{
	std::string ext = GetExtension(pathName);
	if (IsExtensionOk(ext))
	{
		pLogger->Debug("Trying '%s'",pathName.c_str());
		HMODULE hLib = LoadLibrary(pathName.c_str());
		if (hLib != INVALID_HANDLE_VALUE)
		{
			PFNINITIALIZEPLUGIN pFunc;
			pFunc = (PFNINITIALIZEPLUGIN)GetProcAddress(hLib,kDLL_Export);
			if (pFunc != NULL)
			{
				IBaseInstance *pPlugin;
				pLogger->Info("Library '%s' loaded ok, regestring",pathName.c_str());
				pPlugin = ySys->RegisterAndInitializePlugin(pFunc,pathName.c_str());
				pPlugin->AddAttribute("fullpath",pathName.c_str());
				// TODO: Dig out some version stuff and author from DLL resource section
			} else
			{
				pLogger->Warning("Library '%s' failed, unable to find exported function '%s'",pathName.c_str(),kDLL_Export);
			}
		} else
		{
			pLogger->Warning("LoadLibrary failed, skipping..");
		}
	}
}

// assumes that path ends with '\'
void PluginScanner_Win32::ScanDirectory(std::string path)
{
	HANDLE hFIND;
	WIN32_FIND_DATA findFileData;
	std::string searchPath = path.append("\\");
	
	searchPath.append("*.*");
	pLogger->Debug("Scanning: %s",searchPath.c_str());
	hFIND = FindFirstFile(searchPath.c_str(),&findFileData);
	while(FindNextFile(hFIND,&findFileData))
	{
		std::string child = path;
		child.append(findFileData.cFileName);
		if (bRecurse && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if((strcmp(findFileData.cFileName,".")!=0) && (strcmp(findFileData.cFileName, "..")!=0))
			{
				//std::string childPath = path;
				//childPath.append(findFileData.cFileName);
				ScanDirectory(child);
			}
		} else
		{
			//std::string libName = path;
			//libName.append(findFileData.cFileName);
			TryLoadLibrary(child);
		}
	}
	FindClose(hFIND);
}

// implements the plugin scanner for windows
PluginScanner_Win32::PluginScanner_Win32()
{
	pLogger = Logger::GetLogger("PluginScanner_Win32");
}

void PluginScanner_Win32::ScanForPlugins(ISystem *ySys, const char *fullpath, bool bRecurse)
{
	this->bRecurse = bRecurse;
	this->ySys = ySys;
	ScanDirectory(std::string(fullpath));
}
/*-------------------------------------------------------------------------
 File    : $Archive: PluginScanner_Nix.cpp $
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
  test,test
 </pre>
 
 
 \History
 - 17.09.09, FKling, Implementation
 - 23.09.09, FKling, Interface changes to system
 
 ---------------------------------------------------------------------------*/

#include <string>
#include "yapt/logger.h"
#include "yapt/ySystem.h"
#include "yapt/ySystem_internal.h"

#ifdef WIN32 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

using namespace yapt;


// add this to the list of extensions we support
static const char *lExtensions[]=
{
".dll",
".dylib",
NULL
};
// returns the extension of a file name
std::string PluginScanner_Nix::GetExtension(std::string &pathName)
{
	std::string ext;
	int l = pathName.length()-1;
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
bool PluginScanner_Nix::IsExtensionOk(std::string &extension)
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
void PluginScanner_Nix::TryLoadLibrary(std::string &pathName)
{
	std::string ext = GetExtension(pathName);
	if (IsExtensionOk(ext))
	{
		pLogger->Debug("Trying '%s'",pathName.c_str());
		
		void *pLib;
		pLib = dlopen(pathName.c_str(), RTLD_LAZY);
		
		if (pLib != NULL)
		{
			PFNINITIALIZEPLUGIN pFunc;
			pFunc = (PFNINITIALIZEPLUGIN)dlsym(pLib,"yaptInitializePlugin");
			if (pFunc != NULL)
			{
				IBaseInstance *pPlugin;
				pLogger->Info("Library ok, found: 'yaptInitializePlugin'");
				pPlugin = ySys->RegisterAndInitializePlugin(pFunc, pathName.c_str());
				// TODO: Set attributes on Plugin when implemented
				
			} else
			{
				pLogger->Warning("Library failed, skipping...");
			}
		} else
		{
			pLogger->Warning("LoadLibrary failed, skipping..");
		}
	}
}

// assumes that path ends with '\'
void PluginScanner_Nix::ScanDirectory(std::string path)
{
	std::string searchPath = path;
	DIR *pDir = NULL;
	pDir = opendir(path.c_str());
	
	if (pDir == NULL)
	{
		pLogger->Error("Unable to open directory: %s",searchPath.c_str());
		return;
	}
	//	searchPath.append("*.*");
	pLogger->Debug("Scanning: %s",searchPath.c_str());
	
	
	struct dirent *dp;
	while((dp = readdir(pDir)) != NULL)
	{
		struct stat _stat;
		
		std::string entryName(path+"/"+dp->d_name);
		lstat(entryName.c_str(), &_stat);
		
		if (bRecurse && S_ISDIR(_stat.st_mode))
		{
			if((strcmp(dp->d_name,".")!=0) && (strcmp(dp->d_name, "..")!=0))
			{
				ScanDirectory(entryName);
			}
		} else
		{
			TryLoadLibrary(entryName);
		}
	}
	closedir(pDir);
}

// implements the plugin scanner for windows
PluginScanner_Nix::PluginScanner_Nix()
{
	pLogger = Logger::GetLogger("PluginScanner_Nix");
}

void PluginScanner_Nix::ScanForPlugins(ISystem *ySys, const char *fullpath, bool bRecurse)
{
	this->bRecurse = bRecurse;
	this->ySys = ySys;
	ScanDirectory(std::string(fullpath));
}
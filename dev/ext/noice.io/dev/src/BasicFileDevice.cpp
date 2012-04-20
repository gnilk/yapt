/*-------------------------------------------------------------------------
File    : $Archive: BasicFileDevice.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements a device for BasicFileStream's

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  ! Move initialization and registration out of here
</pre>


\History
- 10.11.09, FKling, Refactored out from the BasicFileStream
- 09.12.09, Fkling, Removed dependency of the URL parser
---------------------------------------------------------------------------*/
#include <string>

// stuff from the io library
#include "noice/io/io.h"	
#include "noice/io/ioclasses.h"
#include "noice/io/ioutils.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

//using namespace yapt;
using namespace noice::io;


//////////////////////////////////////////////////////////////////////////
BasicFileStreamDevice::BasicFileStreamDevice()
{
	relpath = "./";
}

void BasicFileStreamDevice::Initialize()
{
	// after set/get param has been organized
	// we don't need to do anything
}

IStream *BasicFileStreamDevice::CreateStream(const char *path, unsigned int flags)
{
	IStream *pStream = NULL;
	std::string fullpath = relpath;

	// Make sure the full ends with a proper path eliminator
#ifdef WIN32
	if(*fullpath.end()!='\\')
	{
		fullpath += "\\";	
	}
#else
	if(*fullpath.end()!='//')
	{
		fullpath += "//";	
	}
#endif
	fullpath += std::string(path);
	pStream = dynamic_cast<IStream *>(new BasicFileStream(fullpath.c_str()));
	return pStream;
}

//////////////////////////////////////////////////////////////////////////
//
// Sets a parameters for the device
// TODO: Enhance this
//
void BasicFileStreamDevice::SetParam(const char *key, const char *value)
{
	if (!strcmp("relpath",key))
	{
		// ILogger *pLogger = Logger::GetLogger("BasicFileStreamDevice");
		//pLogger->Info("New value for %s = %s",key,value);
		relpath = std::string(value);
	}
}

const char *BasicFileStreamDevice::GetParam(const char *key)
{
	if (!strcmp("relpath",key))
	{
		return relpath.c_str();
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

IStreamDevice *BasicFileStreamDeviceFactory::CreateDevice(const char *initparam)
{
	IStreamDevice *pDevice = dynamic_cast<IStreamDevice *>(new BasicFileStreamDevice());
	return pDevice;
}

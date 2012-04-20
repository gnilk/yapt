/*-------------------------------------------------------------------------
File    : $Archive: DeviceFactoryHolder.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Encapsulation of a device factory. Never exposed outside. 
          Used by system when registering a device.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 15.10.09, FKling, Implementation
- 10.11.09, FKling, Moved to IO library

---------------------------------------------------------------------------*/
//#include "yapt/logger.h"
//
//#include "yapt/ySystem.h"
//#include "yapt/ySystem_internal.h"

#include "yapt/io.h"
#include "yapt/ioclasses.h"
#include "yapt/ioutils.h"


#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace noice::io;
//using namespace yapt;


DeviceFactoryHolder::DeviceFactoryHolder(IStreamDeviceFactory *pFactory, const char *url_identifier, const char *initparam) 
{
	this->pFactory = pFactory;
	this->url_identifier = strdup(url_identifier);
	this->initparam = strdup(initparam);

//	AddAttribute(kDefintion_Name, url_identifier);
}

DeviceFactoryHolder::~DeviceFactoryHolder()
{
	free((void *)url_identifier);
	free((void *)initparam);
}

const char *DeviceFactoryHolder::GetURLIdentifier()
{
	return url_identifier;
}

IStreamDevice *DeviceFactoryHolder::CreateDevice()
{
	IStreamDevice *pResult = NULL;
	pResult = pFactory->CreateDevice(initparam);
	//if (pResult == NULL)
	//{
	//	Logger::GetLogger("DeviceFactoryHolder")->Error("Unable to create I/O Device '%s' through factory",url_identifier);
	//} else
	//{
	//	Logger::GetLogger("DeviceFactoryHolder")->Info("I/O Device for '%s' created ok",url_identifier);
	//}

	return pResult;
}
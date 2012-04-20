/*-------------------------------------------------------------------------
File    : $Archive: IODeviceManager.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements device/service handling

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 10.11.09, FKling, Moved here from yapt::system class
- 09.12.09, FKling, Don't pass URL's down to device layer. Rewrote logic when can't find device
---------------------------------------------------------------------------*/

#include <string>
#include <map>

#include "noice/io/io.h"
#include "noice/io/ioclasses.h"
#include "noice/io/ioutils.h"

using namespace noice::io;

static DeviceManager *glb_DeviceManager = NULL;
static BasicFileStreamDeviceFactory glb_BaseFileFactory;	// only built in device..

//
// static method, returns the single instance of the device manager
//
DeviceManager *DeviceManager::GetInstance()
{
	if (glb_DeviceManager == NULL)
	{
		glb_DeviceManager = new DeviceManager();
		// Register the built in "file://" device in the device manager
		glb_DeviceManager->RegisterIODevice(dynamic_cast<IStreamDeviceFactory *>(&glb_BaseFileFactory),"file","", true);
	}

	return glb_DeviceManager;
}

// empty protected constructor
DeviceManager::DeviceManager()
{
	// do nothing
}

//
// Set a device for a specific URL identifier
//
void DeviceManager::SetIODevice(IStreamDevice *pDevice, const char *url_identifier)
{
	// TODO: if already present we should perhaps close the device
	urldevices.insert(UrlDevicePair(url_identifier,pDevice));
}

//
// Enumerates through the devices
//
void DeviceManager::EnumerateURIHandlers(PFNENUMIODEVICE pEnumFunc)
{
	UrlNameDefMap::iterator handler;
	for(handler = devicefactories.begin(); handler != devicefactories.end(); handler++)
	{
		UrlNameDefPair pair = *(handler);
		try
		{
			DeviceFactoryHolder *pHolder = dynamic_cast<DeviceFactoryHolder*>(pair.second);
			if (pHolder != NULL)
			{
				pEnumFunc(pHolder->GetURLIdentifier());
			}
		}catch(...)
		{
			// callback exception
			//pLogger->Error("Callback during EnumURIHandlers caused an exception");			
		}
	}
}

//
// Returns an existing (already opened) io device instance for a url (if any)
//
IStreamDevice *DeviceManager::GetIODevice(const char *url_identifier)
{
	IStreamDevice *pDevice = 0;
	if (urldevices.find(url_identifier) != urldevices.end())
	{
		pDevice = urldevices[url_identifier];
	} else
	{
		//pLogger->Warning("No device created for URL:'%s'",url_identifier);
	}
	return pDevice;
}

//
// Creates an io device
//
IStreamDevice *DeviceManager::CreateIODevice(const char *url_identifier)
{
	IStreamDevice *pResult = NULL;
	if (urldevices.find(url_identifier) != urldevices.end())
	{
		pResult = urldevices[url_identifier];
	} else
	{
		if (devicefactories.find(url_identifier) != devicefactories.end())
		{
			DeviceFactoryHolder *pHolder = devicefactories[url_identifier];
			pResult = pHolder->CreateDevice();
		} else
		{
			//pLogger->Warning("No factory for device with URL '%s'",url_identifier);
		}
	}
	return pResult;
}

//
// Register an io device
//
bool DeviceManager::RegisterIODevice(IStreamDeviceFactory *pDeviceFactory, const char *url_identifier, const char *initparam, bool bCreate)
{
	bool bRes = false;
	if (devicefactories.find(url_identifier) == devicefactories.end())
	{
		DeviceFactoryHolder *pHolder = new DeviceFactoryHolder(pDeviceFactory, url_identifier, initparam);
		devicefactories.insert(UrlNameDefPair(url_identifier, pHolder));
		// Auto create this device upon registration, nice for device not requiring special parameters
		if (bCreate)
		{
			IStreamDevice *pDevice = pHolder->CreateDevice();
			SetIODevice(pDevice, url_identifier);
		}
		bRes = true;
	} else
	{
//		pLogger->Warning("Factory for URL '%s' already registered",url_identifier);
	}
	return bRes;
}

//
// creates a stream from a url
//
IStream *DeviceManager::CreateStream(const char *uri, unsigned int flags)
{
	URLParser *parser = URLParser::ParseURL(uri);
	IStream *result = NULL;
	if (parser != NULL)
	{
		IStreamDevice *pDevice = (IStreamDevice *)urldevices[parser->GetService()];
		if (pDevice != NULL)
		{
			result = pDevice->CreateStream(parser->GetPath(), flags);
		} else
		{
			result = NULL;
		}
		delete parser;
	} else
	{
		// Perhaps we need some protection here in order to not get klogged up
		std::string newname = std::string("file://").append(uri);
		result = CreateStream(newname.c_str(), flags);

		if (!result)
		{
			// SetYaptLastError(kErrorClass_General, kError_URLParseSyntax);
		} else
		{
			// pLogger->Warning("Malformed URI for file, reverted to file:// ok");
		}
	}
	return result;
}

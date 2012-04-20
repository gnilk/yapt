/*-------------------------------------------------------------------------
File    : $Archive: ioutils.h $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Defines the implementation classes.

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 10.11.09, FKling, Refactored out of ySystem_internal.h

---------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <map>

#include "noice/io/io.h"

namespace noice
{
	namespace io
	{
		class URLParser
		{
		protected:
			char *service;
			char *path;

			bool DoParseURL(const char *url);
		public:
			URLParser(const char *url);
			virtual ~URLParser();

			static URLParser *ParseURL(const char *url);

			const char *GetService();
			const char *GetPath();

			char *GetServiceCopy(char *dst, int maxlen);
			char *GetPathCopy(char *dst, int maxlen);
		};

		class DeviceFactoryHolder
		{
		protected:
			IStreamDeviceFactory *pFactory;
			const char *url_identifier;
			const char *initparam;
		public:

			DeviceFactoryHolder(IStreamDeviceFactory *pFactory, const char *url_identifier, const char *initparam);
			virtual ~DeviceFactoryHolder();

			const char *GetURLIdentifier();
			IStreamDevice *CreateDevice();
		};


		typedef std::pair<std::string, DeviceFactoryHolder*> UrlNameDefPair;
		typedef std::map<std::string, DeviceFactoryHolder*> UrlNameDefMap;
		typedef std::pair<std::string, IStreamDevice*> UrlDevicePair;
		typedef std::map<std::string, IStreamDevice*> UrlDeviceMap;


		class DeviceManager
		{
		protected:
			UrlDeviceMap urldevices;
			UrlNameDefMap devicefactories;

			DeviceManager();
		public:
			static DeviceManager *GetInstance();
			void SetIODevice(IStreamDevice *pDevice, const char *url_identifier);
			IStreamDevice *GetIODevice(const char *url_identifier);
			IStreamDevice *CreateIODevice(const char *url_identifier);
			bool RegisterIODevice(IStreamDeviceFactory *pDeviceFactory, const char *url_identifier, const char *initparam, bool bCreate);			
			void EnumerateURIHandlers(PFNENUMIODEVICE pEnumFunc);
			IStream *CreateStream(const char *uri, unsigned int flags);


		};

	}
}

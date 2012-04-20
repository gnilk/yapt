/*-------------------------------------------------------------------------
File    : $Archive: ioclasses.h $
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
#include "noice/io/io.h"


namespace noice
{
	namespace io
	{
		//////////////////////////////////////////////////////////////////////////
		//
		// Stream classes
		//
		class BasicFileStream :
			public IStream
		{
		protected:
			void *handle;
			char *path;
		public:
			BasicFileStream(const char *path);
			virtual ~BasicFileStream();
			virtual bool Open(kStreamOp operation);
			virtual int Write(void *pSource, int nBytesToWrite);
			virtual int Read(void *pDest, int nBytesToRead);
			virtual int Seek(int origin, long relative_offset);
			virtual long Size();
			virtual void Close();
			virtual int Cntl(unsigned int iOptionCode, void *optionData);

		};
		//////////////////////////////////////////////////////////////////////////
		//
		// devices and factories
		//
		class BasicFileStreamDevice :
			public IStreamDevice
		{
		protected:
			std::string relpath;
		public:
			BasicFileStreamDevice();
			virtual IStream *CreateStream(const char *path, unsigned int flags);
			virtual void Initialize();
			virtual void SetParam(const char *key, const char *value);
			virtual const char *GetParam(const char *key);

		};

		class BasicFileStreamDeviceFactory :
			public IStreamDeviceFactory
		{
		public:
			virtual IStreamDevice *CreateDevice(const char *param);
		};

	}
}

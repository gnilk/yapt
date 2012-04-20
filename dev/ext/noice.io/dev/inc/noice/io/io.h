/*-------------------------------------------------------------------------
File    : $Archive: io.h $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : defines the interfaces for the io library

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 10.11.09, FKling, Moved for ySystem.h (refactored out from yapt

---------------------------------------------------------------------------*/

#pragma once

namespace noice
{
	namespace io
	{

		// return codes from IStream::Read, can't be ENUM's since reading functions returns int
		#define kIOStreamError ((int)-2)
		#define	kIOStreamEOF ((int)-1)

		typedef enum
		{
			kStreamOp_Any,
			kStreamOp_ReadOnly,
			kStreamOp_WriteOnly,
			kStreamOp_ReadWrite,
		} kStreamOp;

#ifdef WIN32
#define IO_CALLCONV __stdcall
#else
#define IO_CALLCONV
#endif
		typedef int (IO_CALLCONV *PFNENUMIODEVICE)(const char *url_identifier);

		struct IStream
		{
		public:
			virtual bool Open(kStreamOp operation) = 0;
			virtual int Write(void *pSource, int nBytesToWrite) = 0;
			// must return num bytes read or 0 and set last error
			virtual int Read(void *pDest, int nBytesToRead) = 0;
			virtual int Seek(int origin, long relative_offset) = 0;
			virtual long Size() = 0;
			virtual void Close() = 0;
			virtual int Cntl(unsigned int iOptionCode, void *optionData) = 0;
		};

		// defines the interface for a specific device
		struct IStreamDevice
		{
		public:
			// Create a stream on the device
			virtual IStream *CreateStream(const char *url, unsigned int flags) = 0;
			// Control the device, set specific parameters
			virtual void Initialize() = 0;
			virtual void SetParam(const char *key, const char *value) = 0;
			virtual const char *GetParam(const char *key) = 0;
		};

		// provides the ability to create a stream device
		struct IStreamDeviceFactory
		{
		public:
			virtual IStreamDevice *CreateDevice(const char *param) = 0;
		};
	}
}
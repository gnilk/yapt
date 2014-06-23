/*-------------------------------------------------------------------------
File    : $Archive: BasicFileStream.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-10-17, 15:50
Descr   : Implements a regular File stream based IO Handler for URL "file://"
	can be used as an example. 

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
 + Put in a DLL
 ! Fix return codes
 - Set param for device
 - Need to fix error handling
</pre>


\History
- 17.09.09, FKling, Implementation
- 21.09.09, FKling, Implementation of size
- 15.10.09, FKling, Refactored to fit new I/O Device structure 
- 30.10.09, FKling, Destructor called on Close
- 02.11.09, FKling, Corrected opening bug, if already opened you should not return false
- 11.11.09, FKling, Moved to own library

---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <string>

#include "noice/io/io.h"
#include "noice/io/ioclasses.h"
//#include "noice/io/URLParser.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace noice::io;


BasicFileStream::BasicFileStream(const char *path)
{
	this->path = strdup (path);
	handle = NULL;
}

BasicFileStream::~BasicFileStream()
{
	free((void *)path);
}

bool BasicFileStream::Open(kStreamOp operation)
{
	bool bRes = false;
	if (handle == NULL)
	{
		char tmp[32];
		switch(operation)
		{
		case kStreamOp_ReadOnly :
			snprintf(tmp,256,"rb");
			break;
		default:
			snprintf(tmp,256,"wb");
			break;
		}
		
		handle = (void *)fopen(path, tmp);
		if (handle == NULL)
		{
			//Logger::GetLogger("BasicFileStream")->Error("Unable to open file");
			// yapt::SetYaptLastError(kErrorClass_StreamIO, kError_StreamOpenError);
		} else
		{
			bRes = true;
		}
	} else
	{
		//Logger::GetLogger("BasicFileStream")->Warning("file already opened");
		bRes = true;
	}
	return bRes;
	
}
int BasicFileStream::Write(void *pSource, int nBytesToWrite)
{
	int iRes = -1;
	if (handle!=NULL)
	{
		// TODO: check read/write
		iRes = (int)fwrite(pSource, nBytesToWrite, 1, (FILE *)handle);
	}else
	{
		//yapt::SetYaptLastError(kErrorClass_StreamIO,kError_StreamNotOpen);
	}
	return iRes;
}
int BasicFileStream::Read(void *pDest, int nBytesToRead)
{
	int iRes = -1;
	if (handle != NULL)
	{
		iRes = (int)fread(pDest, 1, nBytesToRead, (FILE *)handle);
		if (ferror((FILE *)handle))
		{
			//iRes = kIOStreamError;
			//yapt::SetYaptLastError(kErrorClass_StreamIO, kError_StreamReadError);
		} else if (feof((FILE *)handle))
		{
			//iRes = kIOStreamEOF;
		}
	} else
	{
		//yapt::SetYaptLastError(kErrorClass_StreamIO,kError_StreamNotOpen);
	}
	return iRes;
}

int BasicFileStream::Seek(int origin, long relative_offset)
{	
	int iRes = -1;
	if (handle != NULL)
	{
		iRes = fseek((FILE *)handle, origin, relative_offset);
		if (ferror((FILE *)handle))
		{
			iRes = -1;
		}
	} else
	{
		//yapt::SetYaptLastError(kErrorClass_StreamIO,kError_StreamNotOpen);
	}
	return iRes;
}

long BasicFileStream::Size()
{

	long iRes = -1;
	if (handle != NULL)
	{
		long curPos = ftell((FILE *)handle);
		fseek((FILE *)handle,0,SEEK_END);
		iRes = ftell((FILE *)handle);
		fseek((FILE *)handle,curPos,SEEK_SET);
	}else
	{
		//yapt::SetYaptLastError(kErrorClass_StreamIO,kError_StreamNotOpen);
	}
	return iRes;
}

void BasicFileStream::Close()
{
	if (handle != NULL)
	{
		fclose((FILE *)handle);
	}else
	{
		//yapt::SetYaptLastError(kErrorClass_StreamIO,kError_StreamNotOpen);
	}
	// now we can remove our self..
	delete this;
}

int BasicFileStream::Cntl(unsigned int iOptionCode, void *optionData)
{	
	// we dont support anything..
	return -1;
}



/*-------------------------------------------------------------------------
File    : $Archive: TextReader.cpp $
Author  : $Author: Fkling $
Version : $Revision: 1 $
Orginal : 2009-11-02, 15:50
Descr   : Implements a class that reads text lines from a stream

Modified: $Date: $ by $Author: Fkling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
! Support simple reading of bytes as well as lines
</pre>


\History
- 02.11.09, FKling, Implementation
- 10.11.09, FKling, Support for simple reads (no CRLN conversion)
- 09.12.09, FKling, Free buffer in destructor
---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#define snprintf _snprintf
#include <windows.h>
#else
#endif

//#include "noice/io/ySystem.h"
//#include "noice/io/TextReader.h"
//using namespace yapt;

#include "noice/io/io.h"
#include "noice/io/ioutils.h"
#include "noice/io/TextReader.h"	// TODO: Reconsider this

using namespace noice::io;

const int TextReader::kTextReader_ChunkSize = 1024;

TextReader::TextReader(IStream *pStream)
{
	buffer = NULL;
	this->pStream = pStream;
	if (pStream->Open(kStreamOp_ReadOnly))
	{
		pos = 0;
		buffer = (char *)malloc(kTextReader_ChunkSize);
		bHaveData = true;
	} else
	{
		this->pStream= NULL;
	}
	pos = endpos = 0;
}
TextReader::~TextReader()
{
	// someone deleted us without calling close
	if(pStream != NULL)
	{
		pStream->Close();
	}
	if (buffer != NULL) 
	{
		free(buffer);
	}
}
bool TextReader::Close()
{
	if (pStream != NULL)
	{
		pStream->Close();
		pStream = NULL;
	}
	return true;
}

char *TextReader::FindLine()
{
	// no more data, last returned line was the last available
	if (pos > endpos) return NULL;	

	bool bNeedMoreChars = false;
	// strip of all empty lines - work on pos, update original buffer pointer
	while(((buffer[pos]==0x0a) || (buffer[pos]==0x0d)) && (buffer[pos]!='\0'))
	{
		pos++;
		if (pos>=endpos) 
		{
			bNeedMoreChars = true;
			break;
		}
	}
	// reached the end, only empty lines??
	int idx = pos;
	if (!bNeedMoreChars)
	{
		// dig out one line..
		while((buffer[idx]!=0x0a) && (buffer[idx]!=0x0d) && (buffer[idx]!='\0'))
		{
			idx++;
			if (idx>=endpos) 
			{
				bNeedMoreChars = true;
				break;
			}
		}
	}
	// need more and we have more..
	if ((bNeedMoreChars) && (bHaveData)) return NULL;

	// allocate and copy line
	char *result = (char *)malloc(idx - pos + 1);
	memcpy(result, &buffer[pos], idx - pos);
	result[idx-pos]='\0';
	pos = idx+1;
	return result;
}
void TextReader::MoreData()
{
	if (pos > 0)
	{
		memset(buffer,'X',pos);
		memcpy(buffer, &buffer[pos],endpos-pos);
	}
	int offset = endpos - pos;
	// can't read directly to "buffer"
	long szRead = kTextReader_ChunkSize - offset;
	endpos = pStream->Read(&buffer[offset], szRead);
	if (endpos < szRead)
	{
		// got less?? -> no more data available
		bHaveData = false;
	}
	endpos += offset;
	pos = 0;
}

char *TextReader::ReadLine()
{
	char *res = NULL;
	// todo: read a line from the buffer
	if (pStream != NULL)
	{
		if (pos > 0) res = FindLine();
		if ((res == NULL) && (bHaveData))
		{
			MoreData();
			res = FindLine();
		}
	}
	return res;
}

char *TextReader::ReadLine(char *pDest, int nMax)
{
	char *tmp = ReadLine();
	if (tmp != NULL)
	{
		snprintf(pDest, nMax, "%s", tmp);
		free(tmp);
	} else
	{
		pDest = NULL;
	}
	return pDest;
}
// simply reads data without doing any line magic
int TextReader::Read(char *pDest, int nBytes)
{
	// TODO: Finalize this function
	int res = 0;
	if ((endpos - pos) > nBytes)
	{
		memcpy(pDest, &buffer[pos],nBytes);
		pos += nBytes;
		res = nBytes;
	} else
	{
		int nCopied = 0;
		int nBlock = 0;

		// no data at all, try suck in some..
		if ((endpos-pos) == 0) MoreData();

		// continue as long as we have bytes to consume and there are bytes to consume
		while((nBytes > 0) && ((endpos - pos) != 0))
		{
			nBlock = (endpos - pos);
			if (nBlock > nBytes)
			{
				nBlock = nBytes;
			}
			memcpy(&pDest[nCopied], &buffer[pos],nBlock);
			nCopied += nBlock;
			pos += nBlock;
			nBytes -= nBlock;
			// end of available information, suck in some more
			MoreData();
		}
		res = nCopied;
	}
	return res;
}
